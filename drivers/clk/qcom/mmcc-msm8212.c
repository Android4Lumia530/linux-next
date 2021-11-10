/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021, Ivaylo Ivanov <ivo.ivanov@null.net>
 */

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>

#include <dt-bindings/clock/qcom,mmcc-msm8212.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "reset.h"
#include "gdsc.h"

enum {
	P_XO,
	P_MMPLL0,
	P_EDPLINK,
	P_MMPLL1,
	P_HDMIPLL,
	P_GPLL0,
	P_EDPVCO,
	P_GPLL1,
	P_DSI0PLL,
	P_DSI0PLL_BYTE,
	P_MMPLL2,
	P_MMPLL3,
	P_DSI1PLL,
	P_DSI1PLL_BYTE,
};

static const struct parent_map mmcc_xo_mmpll0_mmpll1_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 }
};

static const char * const mmcc_xo_mmpll0_mmpll1_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
};

static const struct parent_map mmcc_xo_mmpll0_dsi_hdmi_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_HDMIPLL, 4 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL, 2 },
	{ P_DSI1PLL, 3 }
};

static const char * const mmcc_xo_mmpll0_dsi_hdmi_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"hdmipll",
	"mmss_gpll0_vote",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_mmpll0_1_3_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 },
	{ P_MMPLL3, 3 }
};

static const char * const mmcc_xo_mmpll0_1_3_gpll0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
	"mmpll3",
};

static const struct parent_map mmcc_xo_mmpll0_1_gpll1_0_map[] = {
	{ P_XO, 0 },
	{ P_MMPLL0, 1 },
	{ P_MMPLL1, 2 },
	{ P_GPLL0, 5 },
	{ P_GPLL1, 4 }
};

static const char * const mmcc_xo_mmpll0_1_gpll1_0[] = {
	"xo",
	"mmpll0_vote",
	"mmpll1_vote",
	"mmss_gpll0_vote",
	"gpll1_vote",
};

static const struct parent_map mmcc_xo_dsi_hdmi_edp_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_EDPVCO, 5 },
	{ P_DSI0PLL, 1 },
	{ P_DSI1PLL, 2 }
};

static const char * const mmcc_xo_dsi_hdmi_edp[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"edp_vco_div",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_dsi_hdmi_edp_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL, 1 },
	{ P_DSI1PLL, 2 }
};

static const char * const mmcc_xo_dsi_hdmi_edp_gpll0[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"gpll0_vote",
	"dsi0pll",
	"dsi1pll",
};

static const struct parent_map mmcc_xo_dsibyte_hdmi_edp_gpll0_map[] = {
	{ P_XO, 0 },
	{ P_EDPLINK, 4 },
	{ P_HDMIPLL, 3 },
	{ P_GPLL0, 5 },
	{ P_DSI0PLL_BYTE, 1 },
	{ P_DSI1PLL_BYTE, 2 }
};

static const char * const mmcc_xo_dsibyte_hdmi_edp_gpll0[] = {
	"xo",
	"edp_link_clk",
	"hdmipll",
	"gpll0_vote",
	"dsi0pllbyte",
	"dsi1pllbyte",
};

static struct clk_pll mmpll0 = {
	.l_reg = 0x0004,
	.m_reg = 0x0008,
	.n_reg = 0x000c,
	.config_reg = 0x0014,
	.mode_reg = 0x0000,
	.status_reg = 0x001c,
	.status_bit = 17,
        .clkr.hw.init = &(struct clk_init_data){
                .name = "mmpll0",
                .parent_names = (const char *[]){ "xo" },
                .num_parents = 1,
                .ops = &clk_pll_ops,
        },
};

static struct clk_regmap mmpll0_vote = {
	.enable_reg = 0x0100,
	.enable_mask = BIT(0),
	.hw.init = &(struct clk_init_data){
		.name = "mmpll0_vote",
		.parent_names = (const char *[]){ "mmpll0" },
		.num_parents = 1,
		.ops = &clk_pll_vote_ops,
	},
};

static struct clk_pll mmpll1 = {
	.l_reg = 0x4104,
	.m_reg = 0x4108,
	.n_reg = 0x410c,
	.config_reg = 0x4110,
	.mode_reg = 0x4100,
	.status_reg = 0x411c,
        .clkr.hw.init = &(struct clk_init_data){
                .name = "mmpll1",
                .parent_names = (const char *[]){ "xo" },
                .num_parents = 1,
                .ops = &clk_pll_ops,
        },
};

static struct clk_rcg2 mmss_ahb_clk_src = {
	.cmd_rcgr = 0x5000,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmss_ahb_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_mmss_axi_clk[] = {
	F( 19200000, P_XO, 1, 0, 0),
	F( 37500000, P_GPLL0, 16, 0, 0),
	F( 50000000, P_GPLL0, 12, 0, 0),
	F( 75000000, P_GPLL0, 8, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(150000000, P_GPLL0, 4, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
};

static struct clk_rcg2 mmss_axi_clk_src = {
	.cmd_rcgr = 0x5040,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_mmss_axi_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mmss_axi_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_csi0_1_clk[] = {
	F(100000000, P_GPLL0, 6, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	{ }
};

static struct clk_rcg2 csi0_clk_src = {
	.cmd_rcgr = 0x3090,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 csi1_clk_src = {
	.cmd_rcgr = 0x3100,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_oxili_gfx3d_clk[] = {
	F( 19200000, P_XO,  1, 0, 0),
	F( 37500000, P_GPLL0, 16, 0, 0),
	F( 50000000, P_GPLL0, 12, 0, 0),
	F( 75000000, P_GPLL0, 8, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(150000000, P_GPLL0, 4, 0, 0),
	F(200000000, P_GPLL0, 3, 0, 0),
	F(300000000, P_GPLL0, 2, 0, 0),
	F(400000000, P_MMPLL1, 3, 0, 0),
	{ }
};

static struct clk_rcg2 gfx3d_clk_src = {
	.cmd_rcgr = 0x4000,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_oxili_gfx3d_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "gfx3d_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_vfe0_clk[] = {
	F(37500000, P_GPLL0, 16, 0, 0),
	F(50000000, P_GPLL0, 12, 0, 0),
	F(60000000, P_GPLL0, 10, 0, 0),
	F(80000000, P_GPLL0, 7.5, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(109090000, P_GPLL0, 5.5, 0, 0),
	F(133330000, P_GPLL0, 4.5, 0, 0),
	F(200000000, P_GPLL0, 3, 0, 0),
	F(228570000, P_MMPLL0, 3.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	{ }
};

static struct clk_rcg2 vfe0_clk_src = {
	.cmd_rcgr = 0x3600,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_vfe0_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "vfe0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

/*static struct freq_tbl ftbl_dsi_mdp_clk[] = {
	F(37500000, P_GPLL0, 16, 0, 0),
	F(60000000, P_GPLL0, 10, 0, 0),
	F(75000000, P_GPLL0, 8, 0, 0),
	F(85710000, P_GPLL0, 7, 0, 0),
	F(100000000, P_GPLL0, 6, 0, 0),
	F(133330000, P_MMPLL0, 6, 0, 0),
	F(160000000, P_MMPLL0, 5, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	F(228570000, P_MMPLL0, 3.5, 0, 0),
	F(240000000, P_GPLL0, 2.5, 0, 0),
	F(266670000, P_MMPLL0, 3, 0, 0),
	F(320000000, P_MMPLL0, 2.5, 0, 0),
	{ }
};*/

/*static struct clk_rcg2 mdp_clk_src = {
	.cmd_rcgr = 0x2040,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_dsi_hdmi_gpll0_map,
	.freq_tbl = ftbl_dsi_mdp_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mdp_clk_src",
		.parent_names = mmcc_xo_mmpll0_dsi_hdmi_gpll0,
		.num_parents = 6,
		.ops = &clk_rcg2_ops,
	},
};*/

static struct clk_rcg2 pclk0_clk_src = {
	.cmd_rcgr = 0x2000,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsi_hdmi_edp_gpll0_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "pclk0_clk_src",
		.parent_names = mmcc_xo_dsi_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_rcg2 dsi_clk_src = {
	.cmd_rcgr = 0x2020,
	.mnd_width = 8,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsi_hdmi_edp_gpll0_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "dsi_clk_src",
		.parent_names = mmcc_xo_dsi_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_pixel_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct freq_tbl ftbl_camss_mclk0_1_clk[] = {
	F(24000000, P_GPLL0, 5, 1, 5),
	F(66670000, P_GPLL0, 9, 0, 0),
	{ }
};

static struct clk_rcg2 mclk0_clk_src = {
	.cmd_rcgr = 0x3360,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_mclk0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk0_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 mclk1_clk_src = {
	.cmd_rcgr = 0x3390,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_mclk0_1_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mclk1_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_camss_csi0_1phytimer_clk[] = {
	F(100000000, P_GPLL0, 6, 0, 0),
	F(200000000, P_MMPLL0, 4, 0, 0),
	{ }
};

static struct clk_rcg2 csi0phytimer_clk_src = {
	.cmd_rcgr = 0x3000,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_1phytimer_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi0phytimer_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_rcg2 csi1phytimer_clk_src = {
	.cmd_rcgr = 0x3030,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_camss_csi0_1phytimer_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "csi1phytimer_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl byte0_freq_tbl[] = {
	{ .src = P_DSI0PLL_BYTE },
	{ }
};

static struct clk_rcg2 byte0_clk_src = {
	.cmd_rcgr = 0x2120,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsibyte_hdmi_edp_gpll0_map,
	.freq_tbl = byte0_freq_tbl,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "byte0_clk_src",
		.parent_names = mmcc_xo_dsibyte_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_byte2_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct freq_tbl ftbl_dsi_esc0_clk[] = {
	F(19200000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 esc0_clk_src = {
	.cmd_rcgr = 0x2160,
	.hid_width = 5,
	.parent_map = mmcc_xo_dsibyte_hdmi_edp_gpll0_map,
	.freq_tbl = ftbl_dsi_esc0_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "esc0_clk_src",
		.parent_names = mmcc_xo_dsibyte_hdmi_edp_gpll0,
		.num_parents = 6,
		.ops = &clk_rcg2_ops,
	},
};

static struct freq_tbl ftbl_dsi_vsync_clk[] = {
	F(19200000, P_XO, 1, 0, 0),
	{ }
};

static struct clk_rcg2 mdp_vsync_clk_src = {
	.cmd_rcgr = 0x2080,
	.hid_width = 5,
	.parent_map = mmcc_xo_mmpll0_mmpll1_gpll0_map,
	.freq_tbl = ftbl_dsi_vsync_clk,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "mdp_vsync_clk_src",
		.parent_names = mmcc_xo_mmpll0_mmpll1_gpll0,
		.num_parents = 4,
		.ops = &clk_rcg2_ops,
	},
};

static struct clk_branch camss_csi_ahb_clk = {
	.halt_reg = 0x30bc,
	.clkr = {
		.enable_reg = 0x30bc,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi0_clk = {
	.halt_reg = 0x30b4,
	.clkr = {
		.enable_reg = 0x30b4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi0_clk",
			.parent_names = (const char *[]){
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi0phy_clk = {
	.halt_reg = 0x30c4,
	.clkr = {
		.enable_reg = 0x30c4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi0phy_clk",
			.parent_names = (const char *[]){
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi0pix_clk = {
	.halt_reg = 0x30e4,
	.clkr = {
		.enable_reg = 0x30e4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi0pix_clk",
			.parent_names = (const char *[]){
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi0rdi_clk = {
	.halt_reg = 0x30d4,
	.clkr = {
		.enable_reg = 0x30d4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi0rdi_clk",
			.parent_names = (const char *[]){
				"csi0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch camss_csi1_clk = {
	.halt_reg = 0x3124,
	.clkr = {
		.enable_reg = 0x3124,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi1_clk",
			.parent_names = (const char *[]){
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi1phy_clk = {
	.halt_reg = 0x3134,
	.clkr = {
		.enable_reg = 0x3134,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi1phy_clk",
			.parent_names = (const char *[]){
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi1pix_clk = {
	.halt_reg = 0x3154,
	.clkr = {
		.enable_reg = 0x3154,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi1pix_clk",
			.parent_names = (const char *[]){
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi1rdi_clk = {
	.halt_reg = 0x3144,
	.clkr = {
		.enable_reg = 0x3144,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi1rdi_clk",
			.parent_names = (const char *[]){
				"csi1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_csi_vfe0_clk = {
	.halt_reg = 0x3704,
	.clkr = {
		.enable_reg = 0x3704,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_csi_vfe0_clk",
			.parent_names = (const char *[]){
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_mclk0_clk = {
	.halt_reg = 0x3384,
	.clkr = {
		.enable_reg = 0x3384,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_mclk0_clk",
			.parent_names = (const char *[]){
				"mclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_mclk1_clk = {
	.halt_reg = 0x33b4,
	.clkr = {
		.enable_reg = 0x33b4,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_mclk1_clk",
			.parent_names = (const char *[]){
				"mclk1_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_phy0_csi0phytimer_clk = {
	.halt_reg = 0x3024,
	.clkr = {
		.enable_reg = 0x3024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_phy0_csi0phytimer_clk",
			.parent_names = (const char *[]){
				"csi0phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_phy1_csi1phytimer_clk = {
	.halt_reg = 0x3054,
	.clkr = {
		.enable_reg = 0x3054,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_phy1_csi1phytimer_clk",
			.parent_names = (const char *[]){
				"csi1phytimer_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_vfe0_clk = {
	.halt_reg = 0x36a8,
	.clkr = {
		.enable_reg = 0x36a8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_vfe0_clk",
			.parent_names = (const char *[]){
				"vfe0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_vfe0_ahb_clk = {
	.halt_reg = 0x36b8,
	.clkr = {
		.enable_reg = 0x36b8,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_vfe0_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch camss_vfe0_axi_clk = {
	.halt_reg = 0x36bc,
	.clkr = {
		.enable_reg = 0x36bc,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "camss_vfe0_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};


static struct clk_branch dsi_ahb_clk = {
	.halt_reg = 0x2330,
	.clkr = {
		.enable_reg = 0x2330,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "dsi_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch dsi_byte0_clk = {
	.halt_reg = 0x2328,
	.clkr = {
		.enable_reg = 0x233c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "dsi_byte0_clk",
			.parent_names = (const char *[]){
				"byte0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch dsi_esc0_clk = {
	.halt_reg = 0x232c,
	.clkr = {
		.enable_reg = 0x232c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "dsi_esc0_clk",
			.parent_names = (const char *[]){
				"esc0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch dsi_pclk0_clk = {
	.halt_reg = 0x233c,
	.clkr = {
		.enable_reg = 0x233c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "dsi_pclk0_clk",
			.parent_names = (const char *[]){
				"pclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mdp_ahb_clk = {
	.halt_reg = 0x2318,
	.clkr = {
		.enable_reg = 0x2318,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mdp_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mdp_axi_clk = {
	.halt_reg = 0x2314,
	.clkr = {
		.enable_reg = 0x2314,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mdp_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mdp_dsi_clk = {
	.halt_reg = 0x2320,
	.clkr = {
		.enable_reg = 0x2320,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mdp_dsi_clk",
			.parent_names = (const char *[]){
				"pclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mdp_lcdc_clk = {
	.halt_reg = 0x2340,
	.clkr = {
		.enable_reg = 0x2340,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mdp_lcdc_clk",
			.parent_names = (const char *[]){
				"pclk0_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mdp_vsync_clk = {
	.halt_reg = 0x231c,
	.clkr = {
		.enable_reg = 0x231c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mdp_vsync_clk",
			.parent_names = (const char *[]){
				"vsync_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_misc_ahb_clk = {
	.halt_reg = 0x502c,
	.clkr = {
		.enable_reg = 0x502c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_misc_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_mmssnoc_ahb_clk = {
	.halt_reg = 0x5024,
	.clkr = {
		.enable_reg = 0x5024,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mmss_mmssnoc_bto_ahb_clk = {
	.halt_reg = 0x5028,
	.clkr = {
		.enable_reg = 0x5028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_bto_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch mmss_mmssnoc_axi_clk = {
	.halt_reg = 0x506c,
	.clkr = {
		.enable_reg = 0x506c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_mmssnoc_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch mmss_s0_axi_clk = {
	.halt_reg = 0x5064,
	.clkr = {
		.enable_reg = 0x5064,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "mmss_s0_axi_clk",
			.parent_names = (const char *[]){
				"mmss_axi_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
			.flags = CLK_IGNORE_UNUSED,
		},
	},
};

static struct clk_branch oxili_gfx3d_clk = {
	.halt_reg = 0x4028,
	.clkr = {
		.enable_reg = 0x4028,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "oxili_gfx3d_clk",
			.parent_names = (const char *[]){
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch oxili_ahb_clk = {
	.halt_reg = 0x403c,
	.clkr = {
		.enable_reg = 0x403c,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "oxili_ahb_clk",
			.parent_names = (const char *[]){
				"mmss_ahb_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_branch gmem_gfx3d_clk = {
	.halt_reg = 0x4038,
	.clkr = {
		.enable_reg = 0x4038,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "gmem_gfx3d_clk",
			.parent_names = (const char *[]){
				"gfx3d_clk_src",
			},
			.num_parents = 1,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct gdsc camss_vfe_gdsc = {
	.gdscr = 0x36a4,
	.cxcs = (unsigned int []){ 0x36a8, 0x36ac, 0x3704, 0x3714, 0x36b0 },
	.cxc_count = 5,
	.pd = {
		.name = "camss_vfe",
	},
	.pwrsts = PWRSTS_OFF_ON,
};

static struct gdsc oxili_gdsc = {
	.gdscr = 0x4034,
	.cxcs = (unsigned int []){ 0x4038 },
	.cxc_count = 1,
	.pd = {
		.name = "oxili",
	},
	.pwrsts = PWRSTS_OFF_ON,
};

static struct clk_regmap *mmcc_msm8212_clocks[] = {
	[BYTE0_CLK_SRC] = &byte0_clk_src.clkr,
	[CSI0PHYTIMER_CLK_SRC] = &csi0phytimer_clk_src.clkr,
	[CSI0_CLK_SRC] = &csi0_clk_src.clkr,
	[CSI1PHYTIMER_CLK_SRC] = &csi1phytimer_clk_src.clkr,
	[CSI1_CLK_SRC] = &csi1_clk_src.clkr,
	[DSI_CLK_SRC] = &dsi_clk_src.clkr,
	[ESC0_CLK_SRC] = &esc0_clk_src.clkr,
	[GFX3D_CLK_SRC] = &gfx3d_clk_src.clkr,
	[MCLK0_CLK_SRC] = &mclk0_clk_src.clkr,
	[MCLK1_CLK_SRC] = &mclk1_clk_src.clkr,
	[MDP_VSYNC_CLK_SRC] = &mdp_vsync_clk_src.clkr,
	[MMPLL0] = &mmpll0.clkr,
	[MMPLL0_VOTE] = &mmpll0_vote,
	[MMPLL1] = &mmpll1.clkr,
	[MMSS_AHB_CLK_SRC] = &mmss_ahb_clk_src.clkr,
	[MMSS_AXI_CLK_SRC] = &mmss_axi_clk_src.clkr,
	[PCLK0_CLK_SRC] = &pclk0_clk_src.clkr,	
	[VFE0_CLK_SRC] = &vfe0_clk_src.clkr,
	[CAMSS_CSI0PHY_CLK] = &camss_csi0phy_clk.clkr,
	[CAMSS_CSI0PIX_CLK] = &camss_csi0pix_clk.clkr,
	[CAMSS_CSI0RDI_CLK] = &camss_csi0rdi_clk.clkr,
	[CAMSS_CSI0_CLK] = &camss_csi0_clk.clkr,
	[CAMSS_CSI1PHY_CLK] = &camss_csi1phy_clk.clkr,
	[CAMSS_CSI1PIX_CLK] = &camss_csi1pix_clk.clkr,
	[CAMSS_CSI1RDI_CLK] = &camss_csi1rdi_clk.clkr,
	[CAMSS_CSI1_CLK] = &camss_csi1_clk.clkr,
	[CAMSS_CSI_AHB_CLK] = &camss_csi_ahb_clk.clkr,
	[CAMSS_CSI_VFE0_CLK] = &camss_csi_vfe0_clk.clkr,
	[CAMSS_MCLK0_CLK] = &camss_mclk0_clk.clkr,
	[CAMSS_MCLK1_CLK] = &camss_mclk1_clk.clkr,
	[CAMSS_PHY0_CSI0PHYTIMER_CLK] = &camss_phy0_csi0phytimer_clk.clkr,
	[CAMSS_PHY1_CSI1PHYTIMER_CLK] = &camss_phy1_csi1phytimer_clk.clkr,
	[CAMSS_VFE0_CLK] = &camss_vfe0_clk.clkr,
	[CAMSS_VFE_VFE_AHB_CLK] = &camss_vfe0_ahb_clk.clkr,
	[CAMSS_VFE_VFE_AXI_CLK] = &camss_vfe0_axi_clk.clkr,
	[DSI_AHB_CLK] = &dsi_ahb_clk.clkr,
	[DSI_BYTE0_CLK] = &dsi_byte0_clk.clkr,
	[DSI_ESC0_CLK] = &dsi_esc0_clk.clkr,
	[DSI_PCLK0_CLK] = &dsi_pclk0_clk.clkr,
	[MDP_AHB_CLK] = &mdp_ahb_clk.clkr,
	[MDP_AXI_CLK] = &mdp_axi_clk.clkr,
	[MDP_DSI_CLK] = &mdp_dsi_clk.clkr,
	[MDP_LCDC_CLK] = &mdp_lcdc_clk.clkr,
	[MDP_VSYNC_CLK] = &mdp_vsync_clk.clkr,
	[MMSS_MISC_AHB_CLK] = &mmss_misc_ahb_clk.clkr,
	[MMSS_MMSSNOC_AHB_CLK] = &mmss_mmssnoc_ahb_clk.clkr,
	[MMSS_MMSSNOC_BTO_AHB_CLK] = &mmss_mmssnoc_bto_ahb_clk.clkr,
	[MMSS_MMSSNOC_AXI_CLK] = &mmss_mmssnoc_axi_clk.clkr,
	[MMSS_S0_AXI_CLK] = &mmss_s0_axi_clk.clkr,
	[OXILI_AHB_CLK] = &oxili_ahb_clk.clkr,
	[OXILI_GFX3D_CLK] = &oxili_gfx3d_clk.clkr,
	[GMEM_GFX3D_CLK] = &gmem_gfx3d_clk.clkr,
};

static const struct qcom_reset_map mmcc_msm8212_resets[] = {
	/* TODO: Look for more reset clocks */
	[CAMSS_CSI0PHY_BCR] = { 0x30c0 },
	[CAMSS_CSI0PIX_BCR] = { 0x30e0 },
	[CAMSS_CSI0RDI_BCR] = { 0x30d0 },
	[CAMSS_CSI0_BCR] = { 0x30b0 },
	[CAMSS_CSI1PHY_BCR] = { 0x3130 },
	[CAMSS_CSI1PIX_BCR] = { 0x3150 },
	[CAMSS_CSI1RDI_BCR] = { 0x3140 },
	[CAMSS_CSI1_BCR] = { 0x3120 },
	[CAMSS_CSI_VFE0_BCR] = { 0x3700 },
	[CAMSS_MCLK0_BCR] = { 0x3380 },
	[CAMSS_MCLK1_BCR] = { 0x33b0 },
	[CAMSS_PHY0_BCR] = { 0x3020 },
	[CAMSS_PHY1_BCR] = { 0x3050 },
	[CAMSS_VFE_BCR] = { 0x36a0 },
	[DSI_BCR] = { 0x2200 },
	[MDP_DSI_BCR] = { 0x221C },
	[MMSSNOCAHB_BCR] = { 0x5020 },
	[MMSSNOCAXI_BCR] = { 0x5060 },
	[OXILI_BCR] = { 0x4020 },
};

static struct gdsc *mmcc_msm8212_gdscs[] = {
	[CAMSS_VFE_GDSC] = &camss_vfe_gdsc,
	[OXILI_GDSC] = &oxili_gdsc,
};

static const struct regmap_config mmcc_msm8212_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= 0x5104,
	.fast_io	= true,
};

static const struct qcom_cc_desc mmcc_msm8212_desc = {
	.config = &mmcc_msm8212_regmap_config,
	.clks = mmcc_msm8212_clocks,
	.num_clks = ARRAY_SIZE(mmcc_msm8212_clocks),
	.resets = mmcc_msm8212_resets,
	.num_resets = ARRAY_SIZE(mmcc_msm8212_resets),
	.gdscs = mmcc_msm8212_gdscs,
	.num_gdscs = ARRAY_SIZE(mmcc_msm8212_gdscs),
};

static const struct of_device_id mmcc_msm8212_match_table[] = {
	{ .compatible = "qcom,mmcc-msm8212" },
	{ }
};
MODULE_DEVICE_TABLE(of, mmcc_msm8212_match_table);

static int mmcc_msm8212_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct regmap *regmap;
	struct device *dev = &pdev->dev;

	match = of_match_device(mmcc_msm8212_match_table, dev);
	if (!match)
		return -EINVAL;

	regmap = qcom_cc_map(pdev, &mmcc_msm8212_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	return qcom_cc_really_probe(pdev, &mmcc_msm8212_desc, regmap);
}

static struct platform_driver mmcc_msm8212_driver = {
	.probe		= mmcc_msm8212_probe,
	.driver		= {
		.name	= "mmcc-msm8212",
		.of_match_table = mmcc_msm8212_match_table,
	},
};
module_platform_driver(mmcc_msm8212_driver);

MODULE_DESCRIPTION("QCOM MMCC MSM8212 Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mmcc-msm8212");
