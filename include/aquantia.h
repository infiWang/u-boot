/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Aquantia PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2018, 2021 NXP
 * Copyright 2025 gns, co-author: GitHub Copilot
 */

#ifndef __AQUANTIA_H__
#define __AQUANTIA_H__

#include <phy.h>

#ifdef CONFIG_PHY_AQUANTIA
/**
 * aquantia_upload_firmware_from_memory() - Load Aquantia PHY firmware from memory
 * @phydev: PHY device to load firmware to
 * @fw_data: Pointer to firmware data in memory
 * @fw_length: Size of firmware data in bytes
 *
 * This function loads Aquantia PHY firmware from a memory buffer. It performs
 * CRC validation, parses the firmware header, and loads IRAM and DRAM sections
 * to the PHY's internal memory.
 *
 * Return: 0 on success, negative error code on failure
 */
int aquantia_upload_firmware_from_memory(struct phy_device *phydev,
					 const u8 *fw_data, size_t fw_length);
#else
static inline int aquantia_upload_firmware_from_memory(struct phy_device *phydev,
						       const u8 *fw_data, size_t fw_length)
{
	return -ENODEV;
}
#endif

#endif /* __AQUANTIA_H__ */
