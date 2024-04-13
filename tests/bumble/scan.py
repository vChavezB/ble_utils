import asyncio
import os
import logging

import bumble
import click

import bumble.gatt
from bumble.colors import color
from bumble.device import Device
from bumble.transport import open_transport_or_link
from bumble.keys import JsonKeyStore
from bumble.smp import AddressResolver
from bumble.device import Advertisement
from bumble.hci import Address, HCI_Constant, HCI_LE_1M_PHY, HCI_LE_CODED_PHY

adv_queue = asyncio.Queue()


class AdvertisementPrinter:
    def __init__(self, resolver):
        self.resolver = resolver

    def on_advertisement(self, advertisement):
        print("Found advertisement ", advertisement.address)
        asyncio.run_coroutine_threadsafe(adv_queue.put(advertisement), asyncio.get_event_loop())

    def on_advertising_report(self, report):
        print("Found advertisement ", advertisement.address)
        pass


TRANSPORT = "tcp-client:localhost:9000"


def on_packet_received(packet):
    print(packet)

async def check_connection(device: bumble.device):
    """
    Task to send carl data to the M2B cloud
    :return:
    """
    try:
        message: bumble.device.Advertisement = await asyncio.wait_for(adv_queue.get(), timeout=5.0)
        await device.stop_scanning()
        connection: bumble.device.Connection = await device.connect(message.address)
        print("Connected")
        peer = bumble.device.Peer(connection)
        services = await peer.discover_services()
        print("Services")
        uuids = []
        for svc in services:
            uuids.append(svc.uuid)
        await peer.discover_characteristics()

        for svc in services:
            print(svc.uuid)
            for char in svc.characteristics:
                print("\t"+str(char.uuid))
                if char.properties == bumble.gatt.Characteristic.Properties.NOTIFY:
                    await char.subscribe(on_packet_received)


    except asyncio.TimeoutError:
        print("No BLE Devices found")


async def scan(
        passive,
        scan_interval,
        scan_window,
        phy,
        filter_duplicates
):
    async with await open_transport_or_link(TRANSPORT) as (hci_source, hci_sink):
        device = Device.with_hci(
            'Bumble', 'F0:F1:F2:F3:F4:F5', hci_source, hci_sink
        )

        await device.power_on()
        resolving_keys = []

        resolver = AddressResolver(resolving_keys) if resolving_keys else None

        printer = AdvertisementPrinter(resolver)

        device.on('advertisement', printer.on_advertisement)

        if phy is None:
            scanning_phys = [HCI_LE_1M_PHY, HCI_LE_CODED_PHY]
        else:
            scanning_phys = [{'1m': HCI_LE_1M_PHY, 'coded': HCI_LE_CODED_PHY}[phy]]
        print("starting scan")
        await device.start_scanning(
            active=(not passive),
            scan_interval=scan_interval,
            scan_window=scan_window,
            filter_duplicates=filter_duplicates,
            scanning_phys=scanning_phys,
        )

        check_task = asyncio.create_task(check_connection(device))
        await asyncio.gather(check_task)

        await hci_source.wait_for_termination()


# -----------------------------------------------------------------------------
@click.command()
@click.option('--min-rssi', type=int, help='Minimum RSSI value')
@click.option('--passive', is_flag=True, default=False, help='Perform passive scanning')
@click.option('--scan-interval', type=int, default=60, help='Scan interval')
@click.option('--scan-window', type=int, default=60, help='Scan window')
@click.option(
    '--phy', type=click.Choice(['1m', 'coded']), help='Only scan on the specified PHY'
)
@click.option(
    '--filter-duplicates',
    type=bool,
    default=True,
    help='Filter duplicates at the controller level',
)
def main(
        min_rssi,
        passive,
        scan_interval,
        scan_window,
        phy,
        filter_duplicates
):
    logging.basicConfig(level=os.environ.get('BUMBLE_LOGLEVEL', 'WARNING').upper())
    asyncio.run(
        scan(
            passive,
            scan_interval,
            scan_window,
            phy,
            filter_duplicates
        )
    )


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    main()
