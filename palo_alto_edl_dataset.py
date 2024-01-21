import struct
import ipaddress
import argparse
from bs4 import BeautifulSoup
import requests
from concurrent.futures import ThreadPoolExecutor

Range = struct.Struct('III')

def write_ranges_to_file(filename, ranges):
    with open(filename, 'wb') as file:
        for r in ranges:
            file.write(Range.pack(*r))

def save_data(obj):
    global all_data
    data = requests.get(obj['link']).text.split('\n')
    for i in data:
        all_data.append(i)

def get_network_type(ip_address):
    try:
        ip_address_obj = ipaddress.IPv4Address(ip_address)
        return 4
    except ipaddress.AddressValueError:
        try:
            ip_address_obj = ipaddress.IPv6Address(ip_address)
            return 6
        except ipaddress.AddressValueError:
            return None

def parse_args():
    parser = argparse.ArgumentParser(description='Download and process IP ranges from PaloAltoNetworks feeds.')
    parser.add_argument('-o', '--output', default='/etc/nginx/redblock_ranges.bin', help='Output file path')
    parser.add_argument('-t', '--threads', type=int, default=50, help='Number of threads for parallel processing')
    parser.add_argument('-v', '--version', type=int, choices=[4, 6], default=4, help='IP version (4 or 6)')
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()

    ranges_file = args.output
    thread_count = args.threads

    all_data = []
    all_objs = []

    soup = BeautifulSoup(requests.get('https://saasedl.paloaltonetworks.com/feeds.html').text, features="html.parser")
    for i in soup.find_all('a'):
        link = i.get('href')
        if 'ipv4' in link or 'ipv6' in link:
            n = link.replace('https://saasedl.paloaltonetworks.com/', '')
            n = n.replace('ipv4', '')
            n = n.replace('ipv6', '')
            reason = n.replace('/', '_')
            all_objs.append({'link': link, 'reason': reason})

    with ThreadPoolExecutor(thread_count) as executor:
        executor.map(save_data, all_objs)

    clear = list(set(all_data))
    binary_data = []

    for i in clear:
        try:
            network = ipaddress.ip_network(i, strict=False)
            network_type = int(get_network_type(network.broadcast_address))
            if args.version is None or network_type == args.version:
                start = int(network.network_address)
                end = int(network.broadcast_address)
                data = (start, end, network_type)
                if data not in binary_data:
                    binary_data.append(data)
        except Exception as e:
            print(f"Error parsing {e}")

    write_ranges_to_file(ranges_file, binary_data)

    print(f"Total unique IP ranges processed: {len(clear)}")
    print(f"Total IP ranges written to {ranges_file}: {len(binary_data)}")
