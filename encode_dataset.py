import struct
import sys
import ipaddress
import argparse

# Define the Range structure
Range = struct.Struct('III')

def write_ranges_to_file(filename, ranges):
    """
    Write IP ranges to a binary file.

    Args:
        filename (str): The name of the file.
        ranges (list): List of tuples representing IP ranges.
    """
    with open(filename, 'wb') as file:
        for r in ranges:
            file.write(Range.pack(*r))

def read_ranges_from_file(filename):
    """
    Read IP ranges from a binary file.

    Args:
        filename (str): The name of the file.

    Returns:
        list: List of tuples representing IP ranges.
    """
    ranges = []
    with open(filename, 'rb') as file:
        while True:
            data = file.read(Range.size)
            if not data:
                break
            ranges.append(Range.unpack(data))
    return ranges

def get_network_type(ip_address):
    """
    Determine the network type (IPv4 or IPv6) for a given IP address.

    Args:
        ip_address (str): The IP address.

    Returns:
        int: The network type (4 for IPv4, 6 for IPv6, None if invalid).
    """
    try:
        ip_address_obj = ipaddress.ip_address(ip_address)
        return 4 if ip_address_obj.version == 4 else 6
    except ipaddress.AddressValueError:
        return None

def main():
    parser = argparse.ArgumentParser(description='Generate and store IP ranges in binary format for redblock')
    parser.add_argument('input_file', type=str, help='Path to the input file containing IP ranges.', required=True)
    parser.add_argument('output_file', type=str, default="/etc/nginx/redblock_ranges.bin", help='Path to the output file for storing binary IP ranges.')
    parser.add_argument('--ipv4', action='store_true', help='Include IPv4 ranges.')
    parser.add_argument('--ipv6', action='store_true', help='Include IPv6 ranges.')
    args = parser.parse_args()

    if not args.ipv4 and not args.ipv6:
        print("Error: Specify at least one of --ipv4 or --ipv6.")
        sys.exit(1)

    # Read IP ranges from the input file
    dataset = open(args.input_file, 'r').readlines()

    binary_data = []
    for d in dataset:
        i = d.strip()
        try:
            network = ipaddress.ip_network(i, strict=False)
            network_type = get_network_type(str(network.broadcast_address))
            
            if (args.ipv4 and network_type == 4) or (args.ipv6 and network_type == 6):
                start = int(network.network_address)
                end = int(network.broadcast_address)
                data = (start, end, network_type)
                if data not in binary_data:
                    binary_data.append(data)
        except Exception as e:
            print(f"Error parsing {e}")

    # Write IP ranges to the binary file
    write_ranges_to_file(args.output_file, binary_data)

if __name__ == "__main__":
    main()
