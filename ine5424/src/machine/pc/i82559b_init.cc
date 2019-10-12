// EPOS PC AMD PCNet II (Am79C970A) Ethernet NIC Mediator Initialization

#include <machine/machine.h>
#include <machine/pci.h>
#include <system.h>
#include <machine/pc/e100.h>
__BEGIN_SYS



void i82559b::init(unsigned int unit)
{
    /*db<Init, PCNet32>(TRC) << "PCNet32::init(unit=" << unit << ")" << endl;

    // Scan the PCI bus for device
    PCI::Locator loc = PCI::scan(PCI_VENDOR_ID, PCI_DEVICE_ID, unit);
    if(!loc) {
        db<Init, PCNet32>(WRN) << "PCNet32::init: PCI scan failed!" << endl;
        return;
    }

    // Try to enable IO regions and bus master
    PCI::command(loc, PCI::command(loc) | PCI::COMMAND_IO | PCI::COMMAND_MASTER);

    // Get the config space header and check if we got IO and MASTER
    PCI::Header hdr;
    PCI::header(loc, &hdr);
    if(!hdr) {
        db<Init, PCNet32>(WRN) << "PCNet32::init: PCI header failed!" << endl;
        return;
    }
    db<Init, PCNet32>(INF) << "PCNet32::init: PCI header=" << hdr << endl;
    if(!(hdr.command & PCI::COMMAND_IO))
        db<Init, PCNet32>(WRN) << "PCNet32::init: I/O unaccessible!" << endl;
    if(!(hdr.command & PCI::COMMAND_MASTER))
        db<Init, PCNet32>(WRN) << "PCNet32::init: not master capable!" << endl;

    // Get I/O base port
    IO_Port io_port = hdr.region[PCI_REG_IO].phy_addr;
    db<Init, PCNet32>(INF) << "PCNet32::init: I/O port at " << (void *)(int)io_port << endl;

    // Get I/O irq
    IO_Irq irq = hdr.interrupt_line;
    db<Init, PCNet32>(INF) << "PCNet32::init: PCI interrut pin " << hdr.interrupt_pin << " routed to IRQ " << hdr.interrupt_line << endl;

    // Allocate a DMA Buffer for init block, rx and tx rings
    DMA_Buffer * dma_buf = new (SYSTEM) DMA_Buffer(DMA_BUFFER_SIZE);

    // Initialize the device
    PCNet32 * dev = new (SYSTEM) PCNet32(unit, io_port, irq, dma_buf);

    // Register the device
    _devices[unit].device = dev;
    _devices[unit].interrupt = IC::irq2int(irq);

    // Install interrupt handler
    IC::int_vector(_devices[unit].interrupt, &int_handler);

    // Enable interrupts for device
    IC::enable(_devices[unit].interrupt);*/
}

__END_SYS
