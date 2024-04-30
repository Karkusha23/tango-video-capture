# VideoCapture Tango Client
## Описание
Танго-клиент, предназначенный для чтения информации с VideoCaptureDS устройства. Реализован в виде консольного приложения

Клиент читает аргументы Jpeg и ContourInfo устройства, и выводит изображение с данными о границах на изображении с помощью средств OpenCV
### Аргументы командной строки
1. Имя устройства
2. Частота обновления, мс
## Сборка и запуск проекта на Windows
1. Установить компилятор С++ v120 от Microsoft, LibTango 9.3.4 и OpenCV 3.1.0, скачать VideoCaptureLib
2. Создать пустой проект в Visual Studio, скопировать в него файлы из этой папки
3. В свойствах проекта добавить в каталоги включения

```
%libtango_root%\include
%opencv_root%\build\include
%videocapturelib_root%\include
```
  и в каталоги библиотек добавить 
```
%libtango_root\lib%
%opencv_root\build\x64\vc12\lib%
```
, где `%libtango_root%`, `%opencv_root%` и `%videocapturelib_root%` - пути, по котором установлены соответствующие библиотеки

![image](https://github.com/Karkusha23/my-opencv/assets/16138259/a55fb6a2-4abb-460f-a4fa-b4349b5ed654)

4. Прописать в дополнительных зависимостях

```
opencv_world310d.lib
omniORB4_rtd.lib
omniDynamic4_rtd.lib
COS4_rtd.lib
omnithread_rtd.lib
tangod.lib
comctl32.lib
ws2_32.lib
mswsock.lib
advapi32.lib
```

![image](https://github.com/Karkusha23/my-opencv/assets/16138259/03315f81-03d6-4bce-bb3d-6ecaeaa3311e)

5. Собрать проект
6. Скопировать в папку с .exe следующие файлы из папки `%libtango_root\bin%`:
```
COS421_vc13_rtd.dll
libzmq-v120-mt-gd-4_0_5.dll
omniDynamic421_vc13_rtd.dll
omniORB421_vc13_rtd.dll
omnithread40_vc13_rtd.dll
tangod.dll
```
а также файл `opencv_world310d.dll` из папки `%opencv_root\build\x64\vc12\bin%`

7. Запустить .exe с соответствующими аргументами
