// SPDX-License-Identifier: GPL-2.0+
/*
 * Aquantia PHY firmware loading command
 *
 * Copyright 2025 gns, co-author: GitHub Copilot
 */

#include <command.h>
#include <dm.h>
#include <miiphy.h>
#include <phy.h>
#include <aquantia.h>

static int extract_range(char *input, int *plo, int *phi)
{
	char *end;
	*plo = simple_strtol(input, &end, 16);
	if (end == input)
		return -1;

	if ((*end == '-') && *(++end))
		*phi = simple_strtol(end, NULL, 16);
	else if (*end == '\0')
		*phi = *plo;
	else
		return -1;

	return 0;
}

static int extract_phy_range(char *const argv[], int argc, struct mii_dev **bus,
			     struct phy_device **phydev,
			     int *addrlo, int *addrhi)
{
	struct phy_device *dev = *phydev;

	if ((argc < 1) || (argc > 2))
		return -1;

	/* If there are two arguments, it's busname addr */
	if (argc == 2) {
		*bus = miiphy_get_dev_by_name(argv[0]);

		if (!*bus)
			return -1;

		return extract_range(argv[1], addrlo, addrhi);
	}

	/* It must be one argument, here */

	/*
	 * This argument can be one of two things:
	 * 1) Ethernet device name
	 * 2) Just an address (use the previously-used bus)
	 *
	 * We check all buses for a PHY which is connected to an ethernet
	 * device by the given name.  If none are found, we call
	 * extract_range() on the string, and see if it's an address range.
	 */
	dev = mdio_phydev_for_ethname(argv[0]);

	if (dev) {
		*addrlo = *addrhi = dev->addr;
		*bus = dev->bus;

		return 0;
	}

	/* It's an address or nothing useful */
	return extract_range(argv[0], addrlo, addrhi);
}

static int do_aquantia_fw_load(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	struct mii_dev *bus;
	struct phy_device *phydev = NULL;
	ulong fw_addr, fw_size;
	int addr_lo, addr_hi;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	fw_addr = simple_strtoul(argv[1], NULL, 16);
	fw_size = simple_strtoul(argv[2], NULL, 16);

	bus = mdio_get_current_dev();

	if (extract_phy_range(&argv[3], argc - 3, &bus, &phydev, &addr_lo,
			      &addr_hi))
		return CMD_RET_USAGE;

	if (!bus) {
		puts("No MDIO bus found\n");
		return CMD_RET_FAILURE;
	}

	if (addr_lo != addr_hi) {
		printf("Error: Command only supports a single PHY address\n");
		return CMD_RET_FAILURE;
	}

	if (!phydev) {
		phydev = phy_find_by_mask(bus, 1 << addr_lo);
		if (!phydev) {
			printf("Error: PHY device at address %d on bus %s not found\n",
			       addr_lo, bus->name);
			return CMD_RET_FAILURE;
		}
	}

	miiphy_set_current_dev(bus->name);

	printf("Loading Aquantia firmware to PHY %s@%x:\n", bus->name,
	       phydev->addr);
	printf("  Firmware address: 0x%08lx\n", fw_addr);
	printf("  Firmware size: 0x%08lx (%lu bytes)\n", fw_size, fw_size);

	ret = aquantia_upload_firmware_from_memory(phydev, (u8 *)fw_addr,
						   fw_size);

	if (ret) {
		printf("Error: Firmware loading failed with error %d\n", ret);
		return CMD_RET_FAILURE;
	}

	printf("Aquantia firmware loaded successfully\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	aquantia_fw_load, 5, 0, do_aquantia_fw_load,
	"load Aquantia PHY firmware from memory",
	"<fw_addr> <fw_size> <phydev>\n"
	"    fw_addr   - firmware start address in memory (hex)\n"
	"    fw_size   - firmware size in bytes (hex)\n"
	"    <phydev> may be:\n"
	"      <busname> <addr>  (e.g. eth0 0x1c)\n"
	"      <addr>            (e.g. 0x1c)\n"
	"      <eth name>        (e.g. eth1)"
);
