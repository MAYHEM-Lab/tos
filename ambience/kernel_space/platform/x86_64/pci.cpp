//
// Created by fatih on 5/24/21.
//

#include <block_memory_generated.hpp>
#include <file_system_generated.hpp>
#include <lwip/init.h>
#include <tos/ae/registry.hpp>
#include <tos/virtio/block_device.hpp>
#include <tos/virtio/virtio_netif.hpp>
#include <tos/virtio/x86_pci_transport.hpp>
#include <tos/x86_64/pci.hpp>

tos::ae::services::block_memory::sync_server*
init_virtio_blk(tos::virtio::block_device* dev);

tos::ae::services::filesystem::sync_server*
make_littlefs_server(tos::ae::services::block_memory::sync_server* flash);

tos::ae::services::block_memory::sync_server* init_block_partiton(
    tos::ae::services::block_memory::sync_server* dev, int base_block, int block_count);

tos::ae::services::filesystem::sync_server* fs;
tos::virtio::net_if* net;
void init_pci(tos::physical_page_allocator& palloc, tos::ae::registry_base& registry) {
    lwip_init();

    for (auto dev : tos::x86_64::pci::enumerate_pci()) {
        tos::debug::log("PCI Device at",
                        dev.bus(),
                        dev.slot(),
                        (void*)(uintptr_t)dev.header_type(),
                        (void*)(uintptr_t)dev.vendor(),
                        (void*)(uintptr_t)dev.device_id(),
                        (void*)(uintptr_t)dev.class_code(),
                        (void*)(uintptr_t)dev.subclass(),
                        (void*)(uintptr_t)dev.subsys_id(),
                        (void*)(uintptr_t)dev.status(),
                        "IRQ",
                        int(dev.irq_line()),
                        "BAR0",
                        (void*)(uintptr_t)dev.bar0(),
                        "BAR1",
                        (void*)(uintptr_t)dev.bar1(),
                        "BAR2",
                        (void*)(uintptr_t)dev.bar2(),
                        "BAR3",
                        (void*)(uintptr_t)dev.bar3(),
                        "BAR4",
                        (void*)(uintptr_t)dev.bar4(),
                        "BAR5",
                        (void*)(uintptr_t)dev.bar5(),
                        dev.has_capabilities());

        auto irq = dev.irq_line();
        if (dev.vendor() == 0x1AF4) {
            switch (dev.device_id()) {
            case 0x1001: {
                tos::debug::log("Virtio block device");
                if (fs)
                    break;
                auto bd = new tos::virtio::block_device(
                    tos::virtio::make_x86_pci_transport(std::move(dev)));
                bd->initialize(&palloc);
                auto base_serv = init_virtio_blk(bd);
                auto blk_serv =
                    init_block_partiton(base_serv, base_serv->get_block_count() / 2, 100);
                fs = make_littlefs_server(blk_serv);

                registry.register_service("fs", fs);
                break;
            }
            case 0x1000: {
                tos::debug::log("Virtio network device");
                if (net) {
                    break;
                }
                auto nd = new tos::virtio::network_device(
                    tos::virtio::make_x86_pci_transport(std::move(dev)));
                nd->initialize(&palloc);
                tos::debug::log("MTU", nd->mtu(), int(irq));
                tos::debug::log((void*)(uintptr_t)nd->address().addr[0],
                                (void*)(uintptr_t)nd->address().addr[1],
                                (void*)(uintptr_t)nd->address().addr[2],
                                (void*)(uintptr_t)nd->address().addr[3],
                                (void*)(uintptr_t)nd->address().addr[4],
                                (void*)(uintptr_t)nd->address().addr[5]);
                net = new tos::virtio::net_if(nd);
                tos::debug::log(net);
                set_default(*net);
                net->up();
                break;
            }
            default:
                tos::debug::warn("Unknown virtio type:",
                                 (void*)(uintptr_t)dev.device_id());
                break;
            }
        }
    }
}