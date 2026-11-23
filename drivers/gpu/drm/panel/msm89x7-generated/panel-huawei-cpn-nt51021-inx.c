// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 Huawei MediaPad M3 Lite 8 mainline port
// Generated with linux-mdss-dsi-panel-driver-generator from the Huawei lcdkit
// device tree of the MediaPad M3 Lite 8 (CPN)

#include <linux/delay.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct huawei_cpn_nt51021_inx {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data huawei_cpn_nt51021_inx_supplies[] = {
	{ .supply = "vci" },
	{ .supply = "iovcc" },
};

static inline
struct huawei_cpn_nt51021_inx *to_huawei_cpn_nt51021_inx(struct drm_panel *panel)
{
	return container_of_const(panel, struct huawei_cpn_nt51021_inx, panel);
}

static void huawei_cpn_nt51021_inx_reset(struct huawei_cpn_nt51021_inx *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
}

static int huawei_cpn_nt51021_inx_on(struct huawei_cpn_nt51021_inx *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0xa5);
	mipi_dsi_usleep_range(&dsi_ctx, 6000, 7000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa1, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0xf9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0xf2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0xed);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0xe8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa6, 0xe1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa7, 0xde);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa8, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0xd3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0xcf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0xb8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x90, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0xaa);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x4b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8c, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfd, 0x53);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfa, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0xaa);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x17);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x3e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0xac);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0xad);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0xcf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0xcf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x6b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x7c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x95);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0xb4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x39);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x3a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0x5b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0x6b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0xeb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xec, 0xfc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xed, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0xf9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0xfb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xf9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0xfd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf4, 0x1b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf5, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf6, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0x99);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf8, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf9, 0xa7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfa, 0xad);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfd, 0xb4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x17);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x3e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0xb2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0xb6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0xb2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0xb5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0xbf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0xce);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0xe7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0xec);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x7c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x82);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x9f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0xaa);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0xca);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x39);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x3a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0x5b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0x6b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xec, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xed, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf4, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf5, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf6, 0xa8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0xae);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf8, 0xb6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf9, 0xbf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfa, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfd, 0xca);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x17);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x38);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x3e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x8d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0xb4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0xb8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0xbd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0xdf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0xfe);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x97);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x9e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0xa8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0xbc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0xd8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xef);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x39);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x3a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0x5b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0x6b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xec, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xed, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf3, 0x4b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf4, 0x54);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf5, 0xb6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf6, 0xbd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf8, 0xce);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf9, 0xd8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfa, 0xe1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0xed);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfc, 0xef);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfd, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfe, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x00);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);

	return dsi_ctx.accum_err;
}

static int huawei_cpn_nt51021_inx_off(struct huawei_cpn_nt51021_inx *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_usleep_range(&dsi_ctx, 5000, 6000);

	return dsi_ctx.accum_err;
}

static int huawei_cpn_nt51021_inx_prepare(struct drm_panel *panel)
{
	struct huawei_cpn_nt51021_inx *ctx = to_huawei_cpn_nt51021_inx(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(huawei_cpn_nt51021_inx_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	msleep(30);
	huawei_cpn_nt51021_inx_reset(ctx);

	ret = huawei_cpn_nt51021_inx_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(huawei_cpn_nt51021_inx_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int huawei_cpn_nt51021_inx_unprepare(struct drm_panel *panel)
{
	struct huawei_cpn_nt51021_inx *ctx = to_huawei_cpn_nt51021_inx(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = huawei_cpn_nt51021_inx_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(huawei_cpn_nt51021_inx_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode huawei_cpn_nt51021_inx_mode = {
	.clock = (1200 + 64 + 4 + 36) * (1920 + 102 + 2 + 22) * 60 / 1000,
	.hdisplay = 1200,
	.hsync_start = 1200 + 64,
	.hsync_end = 1200 + 64 + 4,
	.htotal = 1200 + 64 + 4 + 36,
	.vdisplay = 1920,
	.vsync_start = 1920 + 102,
	.vsync_end = 1920 + 102 + 2,
	.vtotal = 1920 + 102 + 2 + 22,
	.width_mm = 108,
	.height_mm = 173,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int huawei_cpn_nt51021_inx_get_modes(struct drm_panel *panel,
						      struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &huawei_cpn_nt51021_inx_mode);
}

static const struct drm_panel_funcs huawei_cpn_nt51021_inx_panel_funcs = {
	.prepare = huawei_cpn_nt51021_inx_prepare,
	.unprepare = huawei_cpn_nt51021_inx_unprepare,
	.get_modes = huawei_cpn_nt51021_inx_get_modes,
};

static int huawei_cpn_nt51021_inx_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct huawei_cpn_nt51021_inx *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct huawei_cpn_nt51021_inx, panel,
				   &huawei_cpn_nt51021_inx_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(huawei_cpn_nt51021_inx_supplies),
					    huawei_cpn_nt51021_inx_supplies,
					    &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE;

	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void huawei_cpn_nt51021_inx_remove(struct mipi_dsi_device *dsi)
{
	struct huawei_cpn_nt51021_inx *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id huawei_cpn_nt51021_inx_of_match[] = {
	{ .compatible = "huawei,cpn-nt51021-inx" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, huawei_cpn_nt51021_inx_of_match);

static struct mipi_dsi_driver huawei_cpn_nt51021_inx_driver = {
	.probe = huawei_cpn_nt51021_inx_probe,
	.remove = huawei_cpn_nt51021_inx_remove,
	.driver = {
		.name = "panel-huawei-cpn-nt51021-inx",
		.of_match_table = huawei_cpn_nt51021_inx_of_match,
	},
};
module_mipi_dsi_driver(huawei_cpn_nt51021_inx_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator");
MODULE_DESCRIPTION("DRM driver for INX_NT51021 8P0 VIDEO TFT 1200 x 1920");
MODULE_LICENSE("GPL");
