# Redblock

Another redirector for your offensive operation infrastructure unless you're ready to add 50k network range to your nginx configuration file.

## Overview

![Redblock](/assets/red.png)

Redblock is nginx module for hiding your C2/phishing/etc infrastructure from sandboxes, threat scanners. Easily block IPs associated with hosting, cloud infrastructure, and known sandbox environments.
you can use your own IPRanges Dataset.

## Configuration

Build the module with dependencies.
```bash
make install && make configure && make
```
Final compiled library will be located at `objs/ngx_http_redblock_module.so`.
you will need to add it to `/etc/nginx/nginx.conf`
```nginx
load_module objs/ngx_http_redblock_module.so;
```
dataset location is `/etc/nginx/redblock_ranges.bin` you can change it in the code, you will have to copy the default dataset to that location. 
```bash
cp ./ipv4_ranges.bin /etc/nginx/redblock_ranges.bin
```
You'll need to restart `nginx` every time you rebuild the module.
```bash
service nginx restart
```
now you can see the blocked requests in error log.

## Bring Your Own dataset
you can encode the list of the ip address you want 
```bash
python encode_dataset.py input_file output_file [--ipv4] [--ipv6]
```
or update the current dataset by running 
```bash
python palo_alto_edl_dataset.py
```


