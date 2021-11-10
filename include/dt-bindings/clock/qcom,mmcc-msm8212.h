/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021, Ivaylo Ivanov <ivo.ivanov@null.net>
 */

#ifndef _DT_BINDINGS_CLK_MSM_MMCC_8212_H
#define _DT_BINDINGS_CLK_MSM_MMCC_8212_H

#define BYTE0_CLK_SRC					0
#define CSI0PHYTIMER_CLK_SRC				1
#define CSI0_CLK_SRC					2
#define CSI1PHYTIMER_CLK_SRC				3
#define CSI1_CLK_SRC					4
#define DSI_CLK_SRC					5
#define ESC0_CLK_SRC					6
#define GFX3D_CLK_SRC					7
#define MCLK0_CLK_SRC					8
#define MCLK1_CLK_SRC					9
#define MDP_VSYNC_CLK_SRC				10
#define MMPLL0						11
#define MMPLL0_VOTE					12
#define MMPLL1						13
#define MMSS_AHB_CLK_SRC				14
#define MMSS_AXI_CLK_SRC				15
#define PCLK0_CLK_SRC					16
#define VFE0_CLK_SRC					17
#define CAMSS_CSI0PHY_CLK				18
#define CAMSS_CSI0PIX_CLK				19
#define CAMSS_CSI0RDI_CLK				20
#define CAMSS_CSI0_CLK					21
#define CAMSS_CSI1PHY_CLK				22
#define CAMSS_CSI1PIX_CLK				23
#define CAMSS_CSI1RDI_CLK				24
#define CAMSS_CSI1_CLK					25
#define CAMSS_CSI_AHB_CLK				26
#define CAMSS_CSI_VFE0_CLK				27
#define CAMSS_MCLK0_CLK				28
#define CAMSS_MCLK1_CLK				29
#define CAMSS_PHY0_CSI0PHYTIMER_CLK			30
#define CAMSS_PHY1_CSI1PHYTIMER_CLK			31
#define CAMSS_VFE0_CLK					32
#define CAMSS_VFE_VFE_AHB_CLK				33
#define CAMSS_VFE_VFE_AXI_CLK				34
#define DSI_AHB_CLK					35
#define DSI_BYTE0_CLK					36
#define DSI_ESC0_CLK					37
#define DSI_PCLK0_CLK					38
#define MDP_AHB_CLK					39
#define MDP_AXI_CLK					40
#define MDP_DSI_CLK					41
#define MDP_LCDC_CLK					42
#define MDP_VSYNC_CLK					43
#define MMSS_MISC_AHB_CLK				44
#define MMSS_MMSSNOC_AHB_CLK				45
#define MMSS_MMSSNOC_BTO_AHB_CLK			46
#define MMSS_MMSSNOC_AXI_CLK				47
#define MMSS_S0_AXI_CLK				48
#define OXILI_AHB_CLK					49
#define OXILI_GFX3D_CLK				50
#define GMEM_GFX3D_CLK					51

/* gdscs */
#define CAMSS_VFE_GDSC					0
#define OXILI_GDSC					1

/* BCRs */
#define CAMSS_CSI0PHY_BCR				0
#define CAMSS_CSI0PIX_BCR				1
#define CAMSS_CSI0RDI_BCR				2
#define CAMSS_CSI0_BCR					3
#define CAMSS_CSI1PHY_BCR				4
#define CAMSS_CSI1PIX_BCR				5
#define CAMSS_CSI1RDI_BCR				6
#define CAMSS_CSI1_BCR					7
#define CAMSS_CSI_VFE0_BCR				8
#define CAMSS_MCLK0_BCR				9
#define CAMSS_MCLK1_BCR				10
#define CAMSS_PHY0_BCR					11
#define CAMSS_PHY1_BCR					12
#define CAMSS_VFE_BCR					13
#define DSI_BCR					14
#define MDP_DSI_BCR					15
#define MMSSNOCAHB_BCR					16
#define MMSSNOCAXI_BCR					17
#define OXILI_BCR					18
#endif
