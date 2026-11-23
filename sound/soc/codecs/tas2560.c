// SPDX-License-Identifier: GPL-2.0
//
// ASoC driver for the Texas Instruments TAS2560 mono class-D smart amplifier
//
// Register sequences derived from TI datasheet SLASE86E and TI's downstream
// tas2560 driver (Huawei MediaPad M3 Lite). The on-chip DSP is programmed with
// the static coefficient set used there (speaker protection runs on the host
// downstream and is not implemented here).

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

/* Register map: book 0 only, pages selected through register 0 */
#define TAS2560_REG(page, reg)		((page) * 128 + (reg))

#define TAS2560_PAGE_CTRL		TAS2560_REG(0, 0)
#define TAS2560_SW_RESET		TAS2560_REG(0, 1)
#define TAS2560_DEV_MODE		TAS2560_REG(0, 2)
#define TAS2560_SPK_CTRL		TAS2560_REG(0, 4)
#define TAS2560_SPK_VOL_MASK		GENMASK(3, 0)
#define TAS2560_PWR_CTRL_2		TAS2560_REG(0, 5)
#define TAS2560_PWR_CTRL		TAS2560_REG(0, 7)
#define TAS2560_PWR_MASK		GENMASK(7, 6)
#define TAS2560_PWR_ON			BIT(6)
#define TAS2560_MUTE			BIT(0)
#define TAS2560_SR_CTRL1		TAS2560_REG(0, 8)
#define TAS2560_LOAD			TAS2560_REG(0, 9)
#define TAS2560_SR_CTRL2		TAS2560_REG(0, 13)
#define TAS2560_SR_CTRL3		TAS2560_REG(0, 14)
#define TAS2560_CLK_SEL			TAS2560_REG(0, 15)
#define TAS2560_PLL_SRC_MASK		GENMASK(7, 6)
#define TAS2560_PLL_SRC_BCLK		0
#define TAS2560_PLL_P_MASK		GENMASK(5, 0)
#define TAS2560_PLL_J			TAS2560_REG(0, 16)
#define TAS2560_PLL_LOWF		BIT(7)
#define TAS2560_PLL_J_MASK		GENMASK(5, 0)
#define TAS2560_PLL_D_MSB		TAS2560_REG(0, 17)	/* PLL_DVAL_1: D[13:8] */
#define TAS2560_PLL_D_LSB		TAS2560_REG(0, 18)	/* PLL_DVAL_2: D[7:0] (latches D) */
#define TAS2560_DAI_FMT			TAS2560_REG(0, 20)
#define TAS2560_DAI_WORDLEN_MASK	GENMASK(1, 0)
#define TAS2560_DAI_FMT_MASK		GENMASK(4, 2)
#define TAS2560_DAI_FMT_I2S		(0 << 2)
#define TAS2560_DAI_FMT_DSP		(1 << 2)
#define TAS2560_DAI_FMT_RIGHT_J		(2 << 2)
#define TAS2560_DAI_FMT_LEFT_J		(3 << 2)
#define TAS2560_ASI_CHANNEL		TAS2560_REG(0, 21)
#define TAS2560_ASI_CHANNEL_MASK	GENMASK(1, 0)
#define TAS2560_ASI_OFFSET_1		TAS2560_REG(0, 22)
#define TAS2560_ASI_CFG_1		TAS2560_REG(0, 24)
#define TAS2560_TRISTATE		BIT(1)
#define TAS2560_BCLKINV			BIT(2)
#define TAS2560_WCLKINV			BIT(3)
#define TAS2560_BCLKDIR			BIT(4)
#define TAS2560_WCLKDIR			BIT(5)
#define TAS2560_DIRINV_MASK		GENMASK(5, 1)
#define TAS2560_CLK_ERR_1		TAS2560_REG(0, 33)
#define TAS2560_PWR_STATUS		TAS2560_REG(0, 42)
#define TAS2560_PCM_RATE		TAS2560_REG(0, 54)
#define TAS2560_PCM_RATE_MASK		GENMASK(1, 0)
#define TAS2560_DR_BOOST_REG_2		TAS2560_REG(0, 60)
#define TAS2560_DR_BOOST_REG_1		TAS2560_REG(0, 73)
#define TAS2560_CLK_ERR_2		TAS2560_REG(0, 80)
#define TAS2560_ID			TAS2560_REG(0, 125)
#define TAS2560_BOOK_CTRL		TAS2560_REG(0, 127)

/* DSP coefficient pages */
#define TAS2560_HPF_CUTOFF_CTL1		TAS2560_REG(50, 28)
#define TAS2560_HPF_CUTOFF_CTL2		TAS2560_REG(50, 32)
#define TAS2560_HPF_CUTOFF_CTL3		TAS2560_REG(50, 36)
#define TAS2560_ISENSE_PATH_CTL1	TAS2560_REG(50, 40)
#define TAS2560_ISENSE_PATH_CTL2	TAS2560_REG(50, 44)
#define TAS2560_ISENSE_PATH_CTL3	TAS2560_REG(50, 48)
#define TAS2560_VLIMIT_THRESHOLD1	TAS2560_REG(50, 60)
#define TAS2560_VLIMIT_THRESHOLD2	TAS2560_REG(50, 64)
#define TAS2560_VLIMIT_THRESHOLD3	TAS2560_REG(50, 68)
#define TAS2560_VLIMIT_THRESHOLD4	TAS2560_REG(50, 72)
#define TAS2560_VLIMIT_THRESHOLD5	TAS2560_REG(50, 76)
#define TAS2560_VLIMIT_THRESHOLD6	TAS2560_REG(50, 80)
#define TAS2560_VLIMIT_THRESHOLD7	TAS2560_REG(50, 84)
#define TAS2560_VLIMIT_THRESHOLD8	TAS2560_REG(50, 88)
#define TAS2560_VLIMIT_THRESHOLD10	TAS2560_REG(50, 96)
#define TAS2560_IDLE_CHNL_DETECT	TAS2560_REG(50, 108)
#define TAS2560_BOOST_ON		TAS2560_REG(51, 16)
#define TAS2560_BOOST_OFF		TAS2560_REG(51, 20)
#define TAS2560_BOOST_HEAD		TAS2560_REG(51, 24)
#define TAS2560_BOOST_TABLE_CTRL1	TAS2560_REG(51, 32)
#define TAS2560_BOOST_TABLE_CTRL2	TAS2560_REG(51, 36)
#define TAS2560_BOOST_TABLE_CTRL3	TAS2560_REG(51, 40)
#define TAS2560_BOOST_TABLE_CTRL4	TAS2560_REG(51, 44)
#define TAS2560_BOOST_TABLE_CTRL5	TAS2560_REG(51, 48)
#define TAS2560_BOOST_TABLE_CTRL6	TAS2560_REG(51, 52)
#define TAS2560_BOOST_TABLE_CTRL7	TAS2560_REG(51, 56)
#define TAS2560_BOOST_TABLE_CTRL8	TAS2560_REG(51, 60)
#define TAS2560_BOOST_TABLE_CTRL9	TAS2560_REG(51, 64)
#define TAS2560_THERMAL_FOLDBACK	TAS2560_REG(51, 100)
#define TAS2560_VSENSE_DEL_CTL1		TAS2560_REG(52, 52)
#define TAS2560_VSENSE_DEL_CTL2		TAS2560_REG(52, 56)
#define TAS2560_VSENSE_DEL_CTL3		TAS2560_REG(52, 60)
#define TAS2560_VSENSE_DEL_CTL4		TAS2560_REG(52, 64)
#define TAS2560_VSENSE_DEL_CTL5		TAS2560_REG(52, 68)

#define TAS2560_MAX_REG			TAS2560_REG(53, 127)

struct tas2560_coef {
	unsigned int reg;
	u8 val[4];
};

/* Isense HPF 2 Hz, output HPF all-pass */
static const struct tas2560_coef tas2560_hpf_coefs[] = {
	{ TAS2560_ISENSE_PATH_CTL1, { 0x7f, 0xfb, 0xb5, 0x00 } },
	{ TAS2560_ISENSE_PATH_CTL2, { 0x80, 0x04, 0x4c, 0x00 } },
	{ TAS2560_ISENSE_PATH_CTL3, { 0x7f, 0xf7, 0x6a, 0x00 } },
	{ TAS2560_HPF_CUTOFF_CTL1,  { 0x7f, 0xff, 0xff, 0xff } },
	{ TAS2560_HPF_CUTOFF_CTL2,  { 0x00, 0x00, 0x00, 0x00 } },
	{ TAS2560_HPF_CUTOFF_CTL3,  { 0x00, 0x00, 0x00, 0x00 } },
};

/* Boost tables for an 8 Ohm load */
static const struct tas2560_coef tas2560_8ohm_coefs[] = {
	{ TAS2560_BOOST_ON,          { 0x75, 0xc2, 0x8e, 0x00 } },
	{ TAS2560_BOOST_OFF,         { 0x6e, 0x14, 0x79, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL1, { 0x1e, 0x00, 0x00, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL2, { 0x21, 0x3d, 0x71, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL3, { 0x24, 0x7a, 0xe1, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL4, { 0x27, 0xb8, 0x52, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL5, { 0x2a, 0xf5, 0xc3, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL6, { 0x2e, 0x33, 0x33, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL7, { 0x31, 0x70, 0xa4, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL8, { 0x34, 0xae, 0x14, 0x00 } },
	{ TAS2560_BOOST_TABLE_CTRL9, { 0x00, 0x00, 0x00, 0x00 } },
};

static const struct tas2560_coef tas2560_misc_coefs[] = {
	{ TAS2560_IDLE_CHNL_DETECT,  { 0x00, 0x00, 0x00, 0x00 } },
	{ TAS2560_BOOST_HEAD,        { 0x06, 0x66, 0x66, 0x00 } },
	{ TAS2560_THERMAL_FOLDBACK,  { 0x39, 0x80, 0x00, 0x00 } },
	{ TAS2560_VSENSE_DEL_CTL1,   { 0x3a, 0x46, 0x74, 0x00 } },
	{ TAS2560_VSENSE_DEL_CTL2,   { 0x22, 0xf3, 0x07, 0x00 } },
	{ TAS2560_VSENSE_DEL_CTL3,   { 0x80, 0x77, 0x61, 0x00 } },
	{ TAS2560_VSENSE_DEL_CTL4,   { 0x22, 0xa7, 0xcc, 0x00 } },
	{ TAS2560_VSENSE_DEL_CTL5,   { 0x3a, 0x0c, 0x93, 0x00 } },
};

/* Voltage limiter (speaker protection) */
static const struct tas2560_coef tas2560_vlimit_coefs[] = {
	{ TAS2560_VLIMIT_THRESHOLD1,  { 0x44, 0x0e, 0x56, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD2,  { 0x41, 0xa5, 0xe3, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD3,  { 0x40, 0x94, 0x13, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD4,  { 0x00, 0x22, 0x1d, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD5,  { 0x02, 0x00, 0x00, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD6,  { 0x7c, 0xf7, 0x66, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD7,  { 0x00, 0x88, 0x40, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD8,  { 0x00, 0x06, 0xd3, 0x00 } },
	{ TAS2560_VLIMIT_THRESHOLD10, { 0x33, 0x43, 0x96, 0x00 } },
};

struct tas2560_priv {
	struct device *dev;
	struct regmap *regmap;
	struct gpio_desc *reset_gpio;
	struct regulator *vdd;
	struct mutex lock;
	unsigned int channel;	/* 0 = left slot, 1 = right slot */
	unsigned int dai_fmt;
	unsigned int rate;
	unsigned int bclk;
	unsigned int slot_width;
	u8 asi_cfg_1;
	u8 dai_fmt_reg;
	u8 wordlen;
	u8 spk_vol;
	bool vdd_enabled;
	bool powered;
	bool unmuted;
};

static const struct regmap_range_cfg tas2560_ranges[] = {
	{
		.range_min = 0,
		.range_max = TAS2560_MAX_REG,
		.selector_reg = TAS2560_PAGE_CTRL,
		.selector_mask = 0xff,
		.selector_shift = 0,
		.window_start = 0,
		.window_len = 128,
	},
};

static bool tas2560_volatile(struct device *dev, unsigned int reg)
{
	return true;
}

static const struct regmap_config tas2560_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = TAS2560_MAX_REG,
	.ranges = tas2560_ranges,
	.num_ranges = ARRAY_SIZE(tas2560_ranges),
	.volatile_reg = tas2560_volatile,
	.cache_type = REGCACHE_NONE,
};

static int tas2560_write_coefs(struct tas2560_priv *tas, const struct tas2560_coef *c, int n)
{
	int i, ret;

	for (i = 0; i < n; i++) {
		ret = regmap_bulk_write(tas->regmap, c[i].reg, c[i].val, 4);
		if (ret)
			return ret;
	}
	return 0;
}

static int tas2560_apply_dai_regs(struct tas2560_priv *tas)
{
	int ret;

	ret = regmap_update_bits(tas->regmap, TAS2560_ASI_CFG_1,
				 TAS2560_DIRINV_MASK, tas->asi_cfg_1);
	if (ret)
		return ret;

	return regmap_update_bits(tas->regmap, TAS2560_DAI_FMT,
				  TAS2560_DAI_FMT_MASK | TAS2560_DAI_WORDLEN_MASK,
				  tas->dai_fmt_reg | tas->wordlen);
}

static int tas2560_set_slot_config(struct tas2560_priv *tas)
{
	unsigned int offset;
	int ret;

	switch (tas->dai_fmt) {
	case SND_SOC_DAIFMT_DSP_A:
		offset = tas->channel ? tas->slot_width + 1 : 1;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		offset = tas->channel ? tas->slot_width : 0;
		break;
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
	case SND_SOC_DAIFMT_RIGHT_J:
	default:
		offset = 0;
		break;
	}

	ret = regmap_write(tas->regmap, TAS2560_ASI_OFFSET_1, offset);
	if (ret)
		return ret;

	return regmap_update_bits(tas->regmap, TAS2560_ASI_CHANNEL,
				  TAS2560_ASI_CHANNEL_MASK, tas->channel);
}

/*
 * PLL: pll_clk = (bclk / P) * J.D = rate * 1024 (SLASE86E section 9.3.8)
 */
static int tas2560_set_pll(struct tas2560_priv *tas)
{
	unsigned int pll_clk = tas->rate * 1024;
	unsigned int bclk = tas->bclk;
	unsigned int p, p_start;
	unsigned int pll_in = 0, j = 0, d = 0;
	u8 p_reg, j_val;
	bool found = false;
	int ret;

	if (!bclk || !pll_clk)
		return -EINVAL;

	p_start = max(1U, DIV_ROUND_UP(bclk, 20000000U));
	for (p = p_start; p <= 64; p++) {
		u64 num;
		u32 rem;

		pll_in = bclk / p;
		if (pll_in < 512000)
			break;

		num = (u64)pll_clk * p;
		j = div_u64_rem(num, bclk, &rem);
		d = DIV_ROUND_CLOSEST_ULL((u64)rem * 10000, bclk);
		if (d >= 10000) {
			j++;
			d -= 10000;
		}

		if (j < 1 || j > 63)
			continue;

		if (d == 0) {
			if (pll_in >= 512000 && pll_in <= 20000000) {
				found = true;
				break;
			}
		} else {
			if (pll_in >= 10000000 && pll_in <= 20000000) {
				found = true;
				break;
			}
		}
	}

	if (!found) {
		dev_err(tas->dev, "no valid PLL config for rate=%u bclk=%u\n",
			tas->rate, bclk);
		return -EINVAL;
	}

	p_reg = (p == 64) ? 0 : p;
	ret = regmap_update_bits(tas->regmap, TAS2560_CLK_SEL,
				 TAS2560_PLL_SRC_MASK | TAS2560_PLL_P_MASK,
				 (TAS2560_PLL_SRC_BCLK << 6) | p_reg);
	if (ret)
		return ret;

	j_val = (j & TAS2560_PLL_J_MASK) | (pll_in < 1000000 ? TAS2560_PLL_LOWF : 0);
	ret = regmap_write(tas->regmap, TAS2560_PLL_J, j_val);
	if (ret)
		return ret;

	/* SLASE86E: MSB (0x11) must be written before LSB (0x12), which latches D */
	ret = regmap_write(tas->regmap, TAS2560_PLL_D_MSB, (d >> 8) & 0x3f);
	if (ret)
		return ret;
	return regmap_write(tas->regmap, TAS2560_PLL_D_LSB, d & 0xff);
}

static int tas2560_set_rate(struct tas2560_priv *tas)
{
	static const u8 sr48k[] = { 0x01, 0x08, 0x10 };
	static const u8 sr16k[] = { 0x01, 0x18, 0x20 };
	static const u8 sr8k[]  = { 0x01, 0x30, 0x20 };
	const u8 *v;
	u8 pcm_rate;
	int ret;

	switch (tas->rate) {
	case 48000:
		v = sr48k;
		pcm_rate = 2;
		break;
	case 44100:
		v = sr48k;
		pcm_rate = 2;
		break;
	case 16000:
		v = sr16k;
		pcm_rate = 1;
		break;
	case 8000:
		v = sr8k;
		pcm_rate = 0;
		break;
	default:
		return -EINVAL;
	}

	ret = regmap_update_bits(tas->regmap, TAS2560_PCM_RATE,
				 TAS2560_PCM_RATE_MASK, pcm_rate);
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_SR_CTRL1,
			   tas->rate == 44100 ? 0x11 : v[0]);
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_SR_CTRL2, v[1]);
	if (ret)
		return ret;
	return regmap_write(tas->regmap, TAS2560_SR_CTRL3, v[2]);
}

/* One-time chip initialization; not called per stream start so ALSA controls persist */
static int tas2560_init_chip(struct tas2560_priv *tas)
{
	int ret;

	ret = regmap_write(tas->regmap, TAS2560_SW_RESET, 0x01);
	if (ret)
		return ret;
	usleep_range(2000, 3000);

	ret = regmap_write(tas->regmap, TAS2560_DR_BOOST_REG_1, 0x0c);
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_DR_BOOST_REG_2, 0x33);
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_CLK_ERR_2, 0x21);
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_LOAD, 0x83);	/* 8 Ohm, no boost limit */
	if (ret)
		return ret;
	ret = regmap_write(tas->regmap, TAS2560_DEV_MODE, 0x02);
	if (ret)
		return ret;

	ret = regmap_update_bits(tas->regmap, TAS2560_SPK_CTRL,
				 TAS2560_SPK_VOL_MASK, tas->spk_vol);
	if (ret)
		return ret;

	return tas2560_apply_dai_regs(tas);
}

static int tas2560_power_locked(struct tas2560_priv *tas, bool on)
{
	unsigned int val;
	int ret;

	if (on) {
		if (tas->powered)
			return 0;

		/* Clear sticky brownout/power error before power-up */
		ret = regmap_write(tas->regmap, TAS2560_PWR_CTRL_2, 0x00);
		if (ret)
			return ret;
		ret = tas2560_set_slot_config(tas);
		if (ret)
			return ret;
		ret = tas2560_set_rate(tas);
		if (ret)
			return ret;
		ret = tas2560_set_pll(tas);
		if (ret)
			return ret;

		/* Power up DSP/boost/Class-D muted first; DSP coefficient pages require power on */
		ret = regmap_write(tas->regmap, TAS2560_PWR_CTRL,
				   TAS2560_PWR_ON | TAS2560_MUTE);
		if (ret)
			return ret;
		usleep_range(16000, 17000);

		ret = tas2560_write_coefs(tas, tas2560_misc_coefs,
					  ARRAY_SIZE(tas2560_misc_coefs));
		if (ret)
			return ret;
		ret = tas2560_write_coefs(tas, tas2560_hpf_coefs,
					  ARRAY_SIZE(tas2560_hpf_coefs));
		if (ret)
			return ret;
		ret = tas2560_write_coefs(tas, tas2560_8ohm_coefs,
					  ARRAY_SIZE(tas2560_8ohm_coefs));
		if (ret)
			return ret;
		ret = tas2560_write_coefs(tas, tas2560_vlimit_coefs,
					  ARRAY_SIZE(tas2560_vlimit_coefs));
		if (ret)
			return ret;

		ret = regmap_write(tas->regmap, TAS2560_CLK_ERR_1, 0x0b);
		if (ret)
			return ret;

		tas->powered = true;
		if (tas->unmuted) {
			ret = regmap_update_bits(tas->regmap, TAS2560_PWR_CTRL,
						 TAS2560_MUTE, 0);
			if (ret)
				return ret;
		}
	} else {
		if (!tas->powered)
			return 0;

		ret = regmap_write(tas->regmap, TAS2560_CLK_ERR_1, 0x00);
		if (ret)
			return ret;
		ret = regmap_write(tas->regmap, TAS2560_PWR_CTRL,
				   TAS2560_PWR_ON | TAS2560_MUTE);
		if (ret)
			return ret;
		usleep_range(5000, 6000);
		ret = regmap_write(tas->regmap, TAS2560_PWR_CTRL, TAS2560_MUTE);
		if (ret)
			return ret;
		msleep(10);

		ret = regmap_read(tas->regmap, TAS2560_SPK_CTRL, &val);
		if (!ret)
			tas->spk_vol = val & TAS2560_SPK_VOL_MASK;

		/*
		 * DPCM stops LPAIF BCLK before DAPM PRE_PMD runs, so clearing
		 * PWR_ON alone leaves internal DAC/Class-D/boost blocks awake
		 * (STATUS_POWER 0x2A = 0xf0). SLASE86E 9.6.3 ends power-down
		 * with SW_RESET; re-init preserves SPK_CTRL and DAI settings
		 * while resetting PWR_CTRL_1 and STATUS_POWER to 0x00.
		 */
		ret = tas2560_init_chip(tas);
		if (ret)
			return ret;

		tas->powered = false;
	}

	return 0;
}

static int tas2560_classd_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(component);
	int ret;

	mutex_lock(&tas->lock);
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		ret = tas2560_power_locked(tas, true);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		ret = tas2560_power_locked(tas, false);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&tas->lock);

	return ret;
}

static int tas2560_mute(struct snd_soc_dai *dai, int mute, int direction)
{
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(dai->component);
	int ret = 0;

	mutex_lock(&tas->lock);
	tas->unmuted = !mute;
	if (tas->powered)
		ret = regmap_update_bits(tas->regmap, TAS2560_PWR_CTRL,
					 TAS2560_MUTE, mute ? TAS2560_MUTE : 0);
	mutex_unlock(&tas->lock);

	return ret;
}

static int tas2560_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(dai->component);
	unsigned int wl;
	int bclk, ret;

	switch (params_width(params)) {
	case 16:
		wl = 0;
		break;
	case 20:
		wl = 1;
		break;
	case 24:
		wl = 2;
		break;
	case 32:
		wl = 3;
		break;
	default:
		return -EINVAL;
	}

	bclk = snd_soc_params_to_bclk(params);
	if (bclk < 0)
		return bclk;

	mutex_lock(&tas->lock);
	tas->rate = params_rate(params);
	tas->bclk = bclk;
	tas->slot_width = params_physical_width(params);
	tas->wordlen = wl;

	ret = regmap_update_bits(tas->regmap, TAS2560_DAI_FMT,
				 TAS2560_DAI_WORDLEN_MASK, wl);
	if (!ret)
		ret = tas2560_set_slot_config(tas);
	mutex_unlock(&tas->lock);

	return ret;
}

static int tas2560_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(dai->component);
	u8 cfg = TAS2560_TRISTATE, sfmt;
	unsigned int format = fmt & SND_SOC_DAIFMT_FORMAT_MASK;
	int ret;

	switch (fmt & SND_SOC_DAIFMT_CLOCK_PROVIDER_MASK) {
	case SND_SOC_DAIFMT_CBC_CFC:
		break;
	default:
		dev_err(tas->dev, "only clock consumer mode is supported\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		cfg |= TAS2560_WCLKINV;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		cfg |= TAS2560_BCLKINV;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		cfg |= TAS2560_WCLKINV | TAS2560_BCLKINV;
		break;
	default:
		return -EINVAL;
	}

	switch (format) {
	case SND_SOC_DAIFMT_I2S:
		sfmt = TAS2560_DAI_FMT_I2S;
		break;
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		sfmt = TAS2560_DAI_FMT_DSP;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		sfmt = TAS2560_DAI_FMT_RIGHT_J;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		sfmt = TAS2560_DAI_FMT_LEFT_J;
		break;
	default:
		return -EINVAL;
	}

	mutex_lock(&tas->lock);
	tas->dai_fmt = format;
	tas->asi_cfg_1 = cfg;
	tas->dai_fmt_reg = sfmt;

	ret = regmap_update_bits(tas->regmap, TAS2560_ASI_CFG_1,
				 TAS2560_DIRINV_MASK, cfg);
	if (!ret)
		ret = regmap_update_bits(tas->regmap, TAS2560_DAI_FMT,
					 TAS2560_DAI_FMT_MASK, sfmt);
	if (!ret)
		ret = tas2560_set_slot_config(tas);
	mutex_unlock(&tas->lock);

	return ret;
}

static const struct snd_soc_dai_ops tas2560_dai_ops = {
	.hw_params	= tas2560_hw_params,
	.set_fmt	= tas2560_set_dai_fmt,
	.mute_stream	= tas2560_mute,
	.no_capture_mute = 1,
};

static struct snd_soc_dai_driver tas2560_dai[] = {
	{
		.name = "tas2560-amplifier",
		.id = 0,
		.playback = {
			.stream_name    = "ASI1 Playback",
			.channels_min   = 1,
			.channels_max   = 2,
			.rates	    = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				      SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000,
			.formats    = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE |
				      SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &tas2560_dai_ops,
	},
};

/* SLASE86E Table 63: SPK_CTRL[3:0] digital gain 0 dB .. +15 dB */
static const DECLARE_TLV_DB_SCALE(tas2560_vol_tlv, 0, 100, 0);

static const struct snd_kcontrol_new tas2560_snd_controls[] = {
	SOC_SINGLE_TLV("Amp Volume", TAS2560_SPK_CTRL, 0, 15, 0, tas2560_vol_tlv),
};

static const struct snd_soc_dapm_widget tas2560_dapm_widgets[] = {
	SND_SOC_DAPM_AIF_IN("ASI1", "ASI1 Playback", 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC", NULL, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_OUT_DRV_E("ClassD", SND_SOC_NOPM, 0, 0, NULL, 0,
			       tas2560_classd_event,
			       SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_OUTPUT("OUT"),
};

static const struct snd_soc_dapm_route tas2560_audio_map[] = {
	{ "DAC", NULL, "ASI1" },
	{ "ClassD", NULL, "DAC" },
	{ "OUT", NULL, "ClassD" },
};

static int tas2560_suspend(struct snd_soc_component *component)
{
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(component);
	unsigned int val;
	int ret;

	mutex_lock(&tas->lock);
	ret = regmap_read(tas->regmap, TAS2560_SPK_CTRL, &val);
	if (!ret)
		tas->spk_vol = val & TAS2560_SPK_VOL_MASK;
	if (tas->powered) {
		ret = tas2560_power_locked(tas, false);
		if (ret) {
			mutex_unlock(&tas->lock);
			return ret;
		}
	}
	if (tas->vdd && tas->vdd_enabled) {
		regulator_disable(tas->vdd);
		tas->vdd_enabled = false;
	}
	mutex_unlock(&tas->lock);

	return 0;
}

static int tas2560_resume(struct snd_soc_component *component)
{
	struct tas2560_priv *tas = snd_soc_component_get_drvdata(component);
	int ret;

	mutex_lock(&tas->lock);
	if (tas->vdd && !tas->vdd_enabled) {
		ret = regulator_enable(tas->vdd);
		if (ret) {
			mutex_unlock(&tas->lock);
			return ret;
		}
		tas->vdd_enabled = true;
		usleep_range(1000, 2000);
	}

	ret = tas2560_init_chip(tas);
	mutex_unlock(&tas->lock);

	return ret;
}

static const struct snd_soc_component_driver soc_component_dev_tas2560 = {
	.controls		= tas2560_snd_controls,
	.num_controls		= ARRAY_SIZE(tas2560_snd_controls),
	.dapm_widgets		= tas2560_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(tas2560_dapm_widgets),
	.dapm_routes		= tas2560_audio_map,
	.num_dapm_routes	= ARRAY_SIZE(tas2560_audio_map),
	.suspend		= tas2560_suspend,
	.resume			= tas2560_resume,
	.idle_bias_on		= 1,
	.endianness		= 1,
};

static void tas2560_regulator_disable(void *data)
{
	struct tas2560_priv *tas = data;

	if (tas->vdd && tas->vdd_enabled) {
		regulator_disable(tas->vdd);
		tas->vdd_enabled = false;
	}
}

static int tas2560_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct tas2560_priv *tas;
	unsigned int id;
	int ret;

	tas = devm_kzalloc(dev, sizeof(*tas), GFP_KERNEL);
	if (!tas)
		return -ENOMEM;

	mutex_init(&tas->lock);
	tas->dev = dev;
	tas->dai_fmt = SND_SOC_DAIFMT_I2S;
	tas->asi_cfg_1 = TAS2560_TRISTATE;
	tas->dai_fmt_reg = TAS2560_DAI_FMT_I2S;
	tas->rate = 48000;
	tas->bclk = 48000 * 32;
	tas->slot_width = 16;
	tas->spk_vol = 0x0f;

	tas->regmap = devm_regmap_init_i2c(client, &tas2560_regmap_config);
	if (IS_ERR(tas->regmap))
		return dev_err_probe(dev, PTR_ERR(tas->regmap), "failed to init regmap\n");

	tas->vdd = devm_regulator_get_optional(dev, "vdd");
	if (IS_ERR(tas->vdd)) {
		if (PTR_ERR(tas->vdd) != -ENODEV)
			return dev_err_probe(dev, PTR_ERR(tas->vdd), "failed to get vdd\n");
		tas->vdd = NULL;
	}
	if (tas->vdd) {
		ret = regulator_enable(tas->vdd);
		if (ret)
			return ret;
		tas->vdd_enabled = true;
		ret = devm_add_action_or_reset(dev, tas2560_regulator_disable, tas);
		if (ret)
			return ret;
	}

	/*
	 * The reset line may be shared between the left and right amplifier,
	 * so deassert it here and leave it deasserted across PM.
	 */
	tas->reset_gpio = devm_gpiod_get_optional(dev, "reset",
			GPIOD_OUT_LOW | GPIOD_FLAGS_BIT_NONEXCLUSIVE);
	if (IS_ERR(tas->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(tas->reset_gpio), "failed to get reset gpio\n");
	if (tas->reset_gpio) {
		gpiod_set_value_cansleep(tas->reset_gpio, 0);
		usleep_range(2000, 3000);
	}

	of_property_read_u32(dev->of_node, "ti,channel", &tas->channel);
	if (tas->channel > 1)
		return -EINVAL;

	ret = regmap_read(tas->regmap, TAS2560_ID, &id);
	if (ret)
		return dev_err_probe(dev, ret, "failed to read chip id\n");

	ret = tas2560_init_chip(tas);
	if (ret)
		return dev_err_probe(dev, ret, "failed to initialize chip\n");

	dev_info(dev, "TAS2560 id 0x%02x, %s channel\n", id, tas->channel ? "right" : "left");

	i2c_set_clientdata(client, tas);

	return devm_snd_soc_register_component(dev, &soc_component_dev_tas2560,
					       tas2560_dai, ARRAY_SIZE(tas2560_dai));
}

static const struct of_device_id tas2560_of_match[] = {
	{ .compatible = "ti,tas2560" },
	{ }
};
MODULE_DEVICE_TABLE(of, tas2560_of_match);

static const struct i2c_device_id tas2560_id[] = {
	{ "tas2560" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tas2560_id);

static struct i2c_driver tas2560_i2c_driver = {
	.driver = {
		.name = "tas2560",
		.of_match_table = tas2560_of_match,
	},
	.probe = tas2560_probe,
	.id_table = tas2560_id,
};
module_i2c_driver(tas2560_i2c_driver);

MODULE_DESCRIPTION("ASoC TAS2560 driver");
MODULE_LICENSE("GPL");
