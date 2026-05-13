#include "phy.h"
#include "mii.h"

static void mdelay(unsigned long msec)
{
    /* user code begin here */
}

int phy_device_create(phy_device_t * phy_dev)
{

}

/* reset phy */
int phy_reset(phy_device_t * phy_dev)
{
    int reg;
	int timeout = 2000;

    if (phy_dev->flags & PHY_FLAG_BROKEN_RESET)
        return 0;

    /* write reset bit */
    if (0 > phy_write(phy_dev, MII_BCR, MII_BCR_RESET))
        return -1;

    /*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
    do {
        reg = phy_read(phy_dev, MII_BCR);
        if (reg < 0) return -1;
        mdelay(1);
    }while((reg & MII_BCR_RESET) && --timeout);

    /* check if time out */
    if (reg & MII_BCR_RESET)
        return -1;

    return 0;
}

int phy_init(phy_device_t * phy_dev)
{
    // int reg;

    // if (phy_reset(phy_dev))
    //     return -1;
    // /* read BCR register(register 0) */
    // reg = phy_read(phy_dev, MII_BCR);
}

/* read register to fetch phy id */
int get_phy_id(smi_bus_t * bus, int paddr, unsigned int * phy_id)
{
	int phy_reg;

	/* Grab the bits from PHYIR1, and put them
	 * in the upper half */
	phy_reg = bus->read(bus, paddr, MII_PHYSID1);

	if (phy_reg < 0)
		return -1;

	*phy_id = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = bus->read(bus, paddr, MII_PHYSID2);

	if (phy_reg < 0)
		return -1;

	*phy_id |= (phy_reg & 0xffff);

	return 0;
}

/* restart auto-negotiation */
int phy_restart_autoneg(phy_device_t * phy_dev)
{
    int phy_reg = 0;

    phy_reg = phy_read(phy_dev, MII_BCR);

    if (phy_reg < 0)
        return -1;
    
    /* enable auto-negotiation and restart it */
    phy_reg |= MII_BCR_AUTO_NEG | MII_BCR_RESTART_AUTO_NEG;

    /* Don't isolate the PHY if we're negotiating */
	phy_reg &= ~(MII_BCR_ISOLATE);

    return phy_write(phy_dev, MII_BCR, phy_reg);
}

int phy_update_link(phy_device_t * phy_dev)
{

}