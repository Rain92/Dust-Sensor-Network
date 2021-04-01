from os.path import expanduser, join
from subprocess import Popen, PIPE

home = expanduser("~")
piodir = join(home, ".platformio/penv/Scripts/")
uploadcmd = piodir + "pio run -e OTA -t upload --upload-port {}"

# IP adresses of the sensors to update
sensor_ip_base = "192.168.43."
sensor_ip_suffixes = [137, 167, 192, 91,
                      27, 70, 219, 149,
                      140, ]

sensor_ips = list(map(lambda s: sensor_ip_base + str(s), sensor_ip_suffixes))


def execute(cmd):
    popen = Popen(cmd.split(' '), stdout=PIPE, universal_newlines=True)
    for stdout_line in iter(popen.stdout.readline, ""):
        print(stdout_line, end="")
    popen.stdout.close()
    popen.wait()
    return popen.returncode


for ip in sensor_ips:
    cmd = uploadcmd.format(ip)
    print("Executing:", cmd)

    ret = execute(cmd)
    print("Return code:", ret)
    if ret != 0:
        break
