import sys
import os
import re
import shutil
import tarfile
import subprocess

import requests


def file_size(num, suffix='B'):
    for unit in ['', 'Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei', 'Zi']:
        if abs(num) < 1024.0:
            return "%3.1f%s%s" % (num, unit, suffix)
        num /= 1024.0
    return "%.1f%s%s" % (num, 'Yi', suffix)


def find_assets(patterns):
    api_url = 'https://api.github.com/repos/metacall/core/releases/latest'
    urls = []
    res = requests.get(api_url)
    data = res.json()
    data = [li['browser_download_url'] for li in data['assets']]
    for p in patterns:
        regex = re.compile(p)
        urls.append(list(filter(regex.search, data))[0])
    return urls


def download(urls):
    for url in urls:
        filename = '/tmp/{}'.format(url.split("/")[-1])

        if os.path.isfile(filename + '.tmp'):
            os.remove(filename + '.tmp')

        with open(filename + '.tmp', 'wb') as file:
            res = requests.get(url, stream=True)
            total_length = res.headers.get('content-length')

            if total_length is None:
                print('Downloading {} from {}\n'.format(url.split("/")[-1], url))
                file.write(res.content)
            else:
                dl = 0
                total_length = int(total_length)
                print('Downloading {} (total: {}) from {}\n'.format(url.split("/")[-1], total_length, url))
                for data in res.iter_content(chunk_size=4096):
                    dl += len(data)
                    file.write(data)
                    done = int(50 * dl / total_length)
                    sys.stdout.write("\r[%s%s]" % ('=' * done, ' ' * (50 - done)))
                    sys.stdout.write(
                        '\r[{}{}] - {}/{}'.format('=' * done, ' ' * (50 - done),
                                                  file_size(dl), file_size(total_length))
                    )
                    sys.stdout.flush()
                print('\n')

        os.rename(filename + '.tmp', filename)


def unpack(files):
    for filename in files:
        filename = filename.split("/")[-1]
        print('Extracting {} ...'.format(filename))
        with tarfile.open(filename) as file:
            file.extractall()


def write_install_log(content):
    with open('/tmp/mc-install.tmp', 'w') as logger:
        logger.write('\n'.join(content))

    shutil.move('/tmp/mc-install.tmp', '/usr/local/share/metacall/install')


def read_install_log():
    with open('/usr/local/share/metacall/install', 'r') as logger:
        lines = logger.read().splitlines()
    return lines


def overwrite(src, dest):
    if os.path.isdir(src):
        if not os.path.isdir(dest):
            os.makedirs(dest)
        files = os.listdir(src)
        for file in files:
            yield from overwrite(os.path.join(src, file), os.path.join(dest, file))
    else:
        print('copying {} to {}'.format(src, dest))
        shutil.copyfile(src, dest)
        yield str(dest)


def spawn(args):
    process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, error = process.communicate()

    return output, error, process.returncode


def install(ignore=False):
    if ignore is False:
        if os.path.isfile('/usr/local/bin/metacallcli'):
            print('MetaCall CLI is already installed')
            exit(0)

    if os.getuid() != 0:
        print('You need to have root privileges to run this script.')
        exit(-1)

    os.chdir('/tmp/')

    docs = ''  # link to documentation if this script failed to download the cli

    try:

        print('Searching for files ...')

        assets = find_assets([r'metacall-[0-9].[0-9].[0.9]-runtime.tar.gz',
                              r'metacall-[0-9].[0-9].[0.9]-examples.tar.gz'])

        missing_files = [file for file in assets if not os.path.isfile(file.split('/')[-1])]

        if len(missing_files) != 0:
            download(missing_files)

        unpack(assets)

        write_install_log(list(overwrite('/tmp/usr/local/', '/usr/local/')))

        print('\n')

        output, error, code = spawn(['ldconfig', '-n', '-v', '/usr/local/lib/'])
        if code != 0:
            print('Failed to install MetaCall\n{}\n'
                  'please proceed with the manual installation. {}'.format(error.decode('utf-8'), docs))
            exit(1)
        else:
            print(output.decode('utf-8'))

        spawn(['chmod', '+x', '/usr/local/bin/metacallcli'])

        print('\nCleaning things up ...')

        shutil.rmtree('/tmp/usr')
        for file in assets:
            path = '/tmp/' + file.split('/')[-1]
            if os.path.isfile(path):
                os.remove(path)

        print('MetaCall CLI is successfully installed.')

    except ConnectionError:
        print('Downloading process failed, please proceed with the manual installation. {}'.format(docs))
    except tarfile.ExtractError:
        print('Extraction process failed, please proceed with the manual installation. {}'.format(docs))
    except KeyboardInterrupt:
        print('\nCanceled by user.')


def update():
    install(ignore=True)


def uninstall(paths):

    for fp in paths:
        if os.path.isfile(fp):
            os.remove(fp)

    if os.path.isdir('/usr/local/share/metacall'):
        shutil.rmtree('/usr/local/share/metacall')

    spawn(['ldconfig'])

    exit(0)


def uninstall_prompt():
    paths = read_install_log()
    message = '''This action would remove:\n   {}
* this action DOES NOT uninstall the python package, only MetaCall CLI and MetaCall libs
* for a complete uninstall you have to run metacall-uninstall && pip uninstall metacall 
* (the order of execution is important)
  
Proceed (y/n)? '''.format(''.join('''{}\n   '''.format(l) for l in paths))

    answers = {'yes': True, 'y': True, 'no': False, 'n': False}

    try:
        while True:
            sys.stdout.write(message)
            choice = input().lower()
            if choice in answers:
                if answers[choice]:
                    uninstall(paths)
                else:
                    exit(0)
            else:
                sys.stdout.write("Please respond with 'yes' or 'no' (or 'y' or 'n').\n")
    except KeyboardInterrupt:
        exit(1)