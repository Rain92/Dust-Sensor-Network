from os.path import expanduser, join
from subprocess import Popen, PIPE

home = expanduser("~")
piodir = join(home, ".platformio/penv/Scripts/")
uploadcmd = piodir + "pio run -e OTA -t upload --upload-port {}"

# IP adresses of the sensors
sensor_ip_base = "192.168.43."
sensor_ip_suffixes = [137, 167, 192, 91,
                      27,  149, 140, 219,
                      70, 12, 49, 124,
                      11, 174, 31, 197,
                      233]  # Windsensor

# sensors to actually update over the air
sensors_to_update = range(len(sensor_ip_suffixes))  # all
# sensors_to_update = range(0, 8) # first 8


def execute(cmd):
    popen = Popen(cmd.split(' '), stdout=PIPE, universal_newlines=True)
    for stdout_line in iter(popen.stdout.readline, ""):
        print(stdout_line, end="")
    popen.stdout.close()
    popen.wait()
    return popen.returncode


for i in sensors_to_update:
    ip = sensor_ip_base + str(sensor_ip_suffixes[i])
    cmd = uploadcmd.format(ip)
    print("Executing:", cmd)

    ret = execute(cmd)
    print("Return code:", ret)
    if ret != 0:
        print(F"Failed to update Sensor {i} with IP {ip}")
        break
