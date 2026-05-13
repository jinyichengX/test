#ifndef PHY_H
#define PHY_H

#ifdef __cplusplus
extern "C" {
#endif

#define PHY_FLAG_BROKEN_RESET	(1 << 0) /* soft reset not supported */

typedef enum {
	PHY_INTERFACE_MODE_MII,
    PHY_INTERFACE_MODE_RMII,
	PHY_INTERFACE_MODE_GMII,
	PHY_INTERFACE_MODE_RGMII,

	PHY_INTERFACE_MODE_MAX,
} phy_interface_t;

typedef struct smi_bus_ctx
{
    void * priv;
    int (* init)(struct smi_bus_ctx * bus, void * priv);
    int (* read)(struct smi_bus_ctx * bus, int paddr, int raddr);
    int (* write)(struct smi_bus_ctx * bus, int paddr, int raddr, unsigned short val);
}smi_bus_t;

typedef struct phy_driver_ctx
{
    void * priv;
    int (* reset)(void * priv);
}phy_driver_t;

typedef struct phy_device_ctx
{
    phy_driver_t * drv;
    smi_bus_t * bus;

    phy_interface_t interface;

    int speed;
    int duplex;

    int addr;           /* phy address in SMI bus */
    int id;             /* phy id, get it from register */

    int link;           /* link status,PHY之间的链路状态：断开 or 链接 */
    int autoneg;        /* if support auto-negotiation 0 : not support 1 : support */

    int phy_id;         /* read from register 3: PHY ID 1 and register 4 : PHY ID 2*/

    int flags;
}phy_device_t;


/* 
***********  SMI frame format  ***********

            Management                frame                fields
        -----------------------------------------------------------------------------------------
        Preamble     Start    Operation    PADDR     RADDR      TA           Data           Idle
        (32 bits)   (2bits)    (2bits)    (5bits)   (5bits)   (2bits)      (16bits)       
-------------------------------------------------------------------------------------------------
Read    1... 1        01         10        ppppp     rrrrr      Z0      dddddddddddddddd      Z
Write   1... 1        01         01        ppppp     rrrrr      10      dddddddddddddddd      Z
*/


static int phy_read(phy_device_t * phy_dev, int raddr)
{
    smi_bus_t * bus = phy_dev->bus;
    if (bus->read) return bus->read(bus, phy_dev->addr, raddr);
    return -1;
}

static int phy_write(phy_device_t * phy_dev, int raddr, unsigned short val)
{
    smi_bus_t * bus = phy_dev->bus;
    if(bus->write) return bus->write(bus, phy_dev->addr, raddr, val);
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif // PHY_H