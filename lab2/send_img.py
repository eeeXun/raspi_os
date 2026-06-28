import argparse
import os
import time


def send_to_serial(file: str, serial: str, baud: int):
    if not os.path.exists(file):
        print(f"File {file} not found")
    if not os.path.exists(serial):
        print(f"Serial {serial} not found")
    size = os.stat(file).st_size
    size_byte = size.to_bytes(4, "little")

    with open(serial, "wb", buffering=0) as tty:
        print(f"Start send size: {size}")
        tty.write(size_byte)
        print("Send size done")

        # Because we print some message on raspberry pi after sending size
        # We need to pause some time before sending kernel
        # If we don't sleep, the first few bytes may be dropped
        time.sleep(0.1)
        print("Start send kernel")
        with open(file, "rb") as f:
            tty.write(f.read())
        print("Send kernel done")


def main():
    parser = argparse.ArgumentParser(
        description="Send a img over serial connection."
    )
    parser.add_argument(
        "-f",
        "--file",
        default="kernel8.img",
        help="The img to send (default: kernel8.img)",
    )
    parser.add_argument(
        "-s",
        "--serial",
        default="/dev/ttyUSB0",
        help="The path to the serial device (default: /dev/ttyUSB0)",
    )
    parser.add_argument(
        "-b", "--baud", default=115200, type=int, help="baud rate"
    )
    args = parser.parse_args()
    send_to_serial(args.file, args.serial, args.baud)


if __name__ == "__main__":
    main()
