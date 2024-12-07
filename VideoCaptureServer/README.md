# VideoCaptureServer
## Описание
HTTP-сервер, реализованный с помощью Oat++, берущий кадры из устройства VideoCaptureDS, и выводящий их в веб-приложении в виде потокового видео

При запросе клиента сервер создает отдельный поток, в котором, подключаясь к нужному устройству VideoCaptureDS, читает его атрибут Jpeg и кодирует полученные изображения в .m3u8 плейлист методами библиотеки ffmpeg. Этот плейлист затем поставляется клиенту

### Доступные url
* ```http://localhost:8000``` - домашняя страница
* ```http://localhost:8000/media/*``` - доступ к файлам сервера (```C:\hlsserver```)
* ```http://localhost:8000/media_no_cache/*``` - доступ к файлам сервера (```C:\hlsserver```), игнорируя кеширование. Используется клиентским скриптом для получения динамически изменяющегося .m3u8 файла плейлиста
* ```http://localhost:8000/device/{family}/{group}/{instance}``` - страница просмотра видеопотока устройства согласно указанному адресу в системе Tango
* ```http://localhost:8000/device/{family}/{group}/{instance}/heartbeat``` - запрос по этому адресу отправляется клиентским скриптом каждые 2 секунды, чтобы уведомить сервер, что ресурс еще просматривается. Если таких запросов для определенного девайса не поступает 10 секунд, сервер закрывает соединение с этим устройством, поскольку в нем нет надобности

## Архитектура

![image](https://github.com/user-attachments/assets/a433c0f6-08b4-47e1-88c1-96b5eac0ab7e)


# Сборка и запуск проекта на Windows
* Установить следующие библиотеки
  + libtango для компилятора v141
  + OpenCV 3.4.1
  + Oat++ 1.3.0
  + ffmpeg
* Добавить в системную переменную PATH пути, по которым находятся .dll файлы установленных библиотек

![image](https://github.com/user-attachments/assets/bbc7f7f8-a24b-44a9-8bc2-82c1a6963c6a)

* Установить VideoCaptureLib
* Установить компилятор Visual Studio 17 (v141)
* Добавить содержимое текущей папки в проект Visual Studio
* В свойствах проекта > Каталоги VC++ > Включаемые каталоги добавить следующие папки
  + ```(%VideoCaptureLib_root%)\include```
  + ```(%ffmpeg_root%)\include```
  + ```(%libtango_root%)\include```
  + ```(%opencv_root%)\build\include```
  + ```(%oatpp_root%)\build\include\oatpp-1.3.0\oatpp```
* В свойствах проекта > Каталоги VC++ > Каталоги библиотек добавить следующие папки
  + ```(%ffmpeg_root%)\lib```
  + ```(%libtango_root%)\lib```
  + ```(%opencv_root%)\build\x64\vc15\lib```
  + ```(%oatpp_root%)\build\lib\oatpp-1.3.0```
* В свойствах проекта > Компоновщик > Ввод > Дополнительные зависимости вставить
```
avcodec.lib
avdevice.lib
avfilter.lib
avformat.lib
avutil.lib
postproc.lib
swresample.lib
swscale.lib
opencv_world3415d.lib
oatpp.lib
Ws2_32.lib
omniORB4_rtd.lib
omniDynamic4_rtd.lib
COS4_rtd.lib
omnithread_rtd.lib
tangod.lib
comctl32.lib
mswsock.lib
advapi32.lib
```
* Скопировать содержание папки ```static``` в ```C:\```
