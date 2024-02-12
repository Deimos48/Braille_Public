import requests
import RPi.GPIO as IO
import converter
import make_sound
import serial
import serial.tools.list_ports
from time import sleep


# init gpio of PI
IO.setwarnings(False)           #отключаем показ любых предупреждений
IO.setmode (IO.BCM)             # мы будем программировать контакты GPIO по их функциональным номерам (BCM), то есть мы будем обращаться к PIN39 как ‘GPIO19’
IO.setup(19,IO.OUT)             # инициализируем GPIO19 в качестве цифрового выхода
IO.setup(13,IO.OUT)             # инициализируем GPIO13 в качестве цифрового выхода
IO.setup(26,IO.IN)              # инициализируем GPIO26 в качестве цифрового входа

# инициализация com-порта
port = "/dev/ttyACM0"  # COM port name
# port = "/dev/ttyACM1"  # COM port name
baudrate = 9600  # baud rate

def get_text_from_speech(sound_file_name):
    STT_URL = '###'
    YC_STT_API_KEY = '###'
    # наш аудиофайл
    with open(sound_file_name, 'rb') as f:
        audio_data = f.read()

    # Создам заголовок с API-ключом для Яндекс.Облака, который пошлем в запросе
    headers = {
        'Authorization': f'Api-Key {###}'
    }

    # Отправляем POST-запрос на сервер Яндекс, который занимается расшифровкой аудио,
    # передав его URL, заголовок и сам файл аудиосообщения
    response = requests.post(STT_URL, headers=headers, data=audio_data)

    # Если запрос к Яндекс.Облаку не удался...
    if not response.ok:
        return None

    # Преобразуем JSON-ответ сервера в объект Python
    result = response.json()
    # Возвращаем текст аудиосообщения
    return result.get('result')


def main():
    
    IO.output(13,False)
    IO.output(19,False)
    ser = serial.Serial(port, baudrate=baudrate)
    Is_for_printer = False
    Printer_is_working = False
    while 1:
        if not Printer_is_working:
            if Is_for_printer:
                IO.output(19,True)
                make_sound.rec_wave('recorded.wav')
                IO.output(19,False)
                IO.output(13,True)
                converter.convert_wav_to_ogg('recorded.wav', 'recorded.ogg')
                recognized_text = get_text_from_speech('recorded.ogg')
                recognized_text = "{" + recognized_text.lower().replace(" абзац ", "#") + "}"
                print(recognized_text)
                ser.write(recognized_text.encode('cp1251'))
                IO.output(13,False)
                Is_for_printer = False
                Printer_is_working = True
            else:
                IO.output(19,True)
                make_sound.rec_wave('recorded.wav')
                IO.output(19,False)
                IO.output(13,True)
                converter.convert_wav_to_ogg('recorded.wav', 'recorded.ogg')
                recognized_text = get_text_from_speech('recorded.ogg')
                recognized_text = recognized_text.lower()
                print(recognized_text)
                if recognized_text == "запись":
                    Is_for_printer = True
                    IO.output(13,False)
                    IO.output(19,True)
                    sleep(0.5)
                    IO.output(19,False)
                    sleep(0.5)
                IO.output(13,False)
        else:
            line = ser.readline().decode('cp1251').strip()
            if line:
                print("Received:", line)
                if line == "}":
                    Printer_is_working = False
        if IO.input(26) == True:
            break
    ser.close()


if _name_ == '_main_':
    main()