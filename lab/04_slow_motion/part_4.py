import serial
import time
import io


def file_to_list(file_path, data_list):
    with open(file_path, 'rb+') as f:
        data = f.readline()
        while data:
            data_list.append(data.decode().replace('\n', ''))
            data = f.readline()
    f.close()
    return data_list


def slowdown(data_list, data_new, rate):
    for data in data_list:
        for i in range(rate):
            data_new.append(data)
    return data_new


def speedup(data_list, data_new, rate):
    for i in range(len(data_list)):
        if i % rate == 0:
            data_new.append(data_list[i])
    return data_new



if __name__ == "__main__":
    print("Initializing...")
    # Write the corresponding test file to the folder one level above the current directory
    path = './test.txt'
    try:
        qtpy = serial.Serial('/dev/tty.usbmodem14201',115200)
        print(qtpy.name)
        sio = io.TextIOWrapper(io.BufferedRWPair(qtpy, qtpy))
    except Exception as e:
        print("Serial can not open,Exception=", e)

    while True:
        print("Choose the mode: R for record and P for replay")
        command = input()
        if command == 'R':
            data_in = []
            print("Start the recording after Green light. Recording time is 10s:")
            qtpy.write(b'r')
            print("Reading from qtpy...")
            start = time.time()
            while time.time() - start < 10:
                line = qtpy.readline()
                line = line.decode().replace(' ', '\n')
                data_in.append(line)
            qtpy.write(b'N')
            print("Finish the recording")
            with open(path, 'wb+') as f:
                for item in data_in:
                    f.write(item.encode('utf-8'))
                f.close()

        if command == 'P':
            data_out = []
            data_final = []
            file_to_list(path, data_out)
            print("Now play the recording:")
            print("Choose the speed mode: normal/SD/SU")
            ans = input()
            print("Input the int rate you want to speed up/down")
            rate = int(input())
            if ans == 'normal':
                data_final = data_out
            if ans == 'SD':
                data_final = slowdown(data_out, data_final,rate)
            if ans == 'SU':
                data_final = speedup(data_out, data_final,rate)
           
            print("Choose the times you want to replay record:")
            t = input()
            t = int(t)
            qtpy.write(b'p')
            for i in range(t):
                for item in data_final:
                    if item == '1\r':
                        qtpy.write(b'1')
                    if item == '0\r':
                        qtpy.write(b'0')
            qtpy.write(b'N')
            print("Finish replay")


        
