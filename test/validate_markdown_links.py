#!/usr/bin/env python
# coding: utf-8

import os
import sys
import yaml
import requests
import logging

import markdown_link_extractor

logging.basicConfig(format='%(message)s')

HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36',
}


def main(argv):
    target = argv[-1]
    if target == argv[0]:
        target = os.getcwd()
    return_code = 0
    for filename in os.listdir(target):
        if filename.endswith('.md'):
            with open(os.path.join(target, filename), 'r') as f:
                links = markdown_link_extractor.getlinks(f.read())
                for link in links:
                    validation = validate_link(link)
                    if not validation['valid']:
                        return_code = 1
                        logging.error("{0}:{1} - Broken link:".format(filename, get_line_number(f, link)))
                        logging.error(yaml.safe_dump(validation, default_flow_style=False))
    return return_code


def validate_link(link):
    # These are just local markdown links, we don't need to validate them like the URLs
    if link.startswith("#"):
        return {"valid": True, "url": link}
    try:
        response = requests.get(link, headers=HEADERS)
    except requests.exceptions.MissingSchema as e:
        return {"valid": False, "reason": "MissingSchema", "url": link, "detail": e.args}
    except Exception as e:
        return {"valid": False, "reason": "Error", "exception": e.__class__.__name__, "url": link, "detail": e.args}

    if response.ok:
        return {"valid": True, "url": link}

    return {
        "valid": False,
        "reason": response.reason,
        "status_code": response.status_code,
        "url": link
    }


def get_line_number(fd, lookup):
    fd.seek(0)
    for num, line in enumerate(fd, 1):
        if lookup in line:
            return num


if __name__ == '__main__':
    sys.exit(main(sys.argv))
