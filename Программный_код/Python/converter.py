# from pydub import AudioSegment
import soundfile as sf


def convert_wav_to_ogg(path_to_wav, path_to_ogg):
    data, samplerate = sf.read(path_to_wav)
    sf.write(path_to_ogg, data, samplerate)

if __name__ == '__main__':
    convert_wav_to_ogg('recorded.wav', 'recorded.ogg')
