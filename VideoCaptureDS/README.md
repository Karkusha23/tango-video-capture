# VideoCapture Tango device
## Описание
Tango-устройство, захватывающее кадр с веб-камеры и обрабатывающее его с помощью методов OpenCV. Передача кадра осуществляется через чтение атрибута Tango-устройства

### Properties
* `Source` (unsigned short) - индекс веб-камеры, с которой захватывается кадр (по умолчанию 0)
* `Mode` (string) - режим камеры. Допустимые значения:
  + `RGB` или `rgb` - в атрибуты записывается RGB-изображение
  + `BGR` или `bgr` - в атрибуты записывается BGR-изображение
  + `Grayscale` или `grayscale` - в атрибуты записывается одноканальное черно-белое изображение
* `Height` (unsigned short) - высота изображения в пикселях (максимум - 720)
* `Width` (unsigned short) - ширина изображения в пикселях (максимум - 1280)
* `JpegQuality` (unsigned short) - качество сжатия в Jpeg (от 0 до 100)
* `Threshold` (unsigned short) - Порог для поиска границ на изображении

_Чтобы изменения свойств вступили в силу, требуется выполнить команду `Reconnect`!_

### Commands
* `State` - Возвращает состояние устройства (Создано автоматически)
* `Status` - Возвращает статус устройства (Создано автоматически)
* `Capture` - Производит захват кадра с веб-камеры и его обработку, ничего не возвращает
* `Reconnect` - Переподключение к веб-камере, ничего не возвращает

### Attributes
* `Frame` (image of unsigned short) - read only. В этот атрибут записывается несжатое изображение с веб-камеры при выполнении команды `Capture`
* `Jpeg` (scalar encoded) - read only. В этот атрибут записывается jpeg-изображение с веб-камеры при выполнении команды `Capture`
* `ContourInfo` (scalar encoded) - read only. В этот атрибут записываются данные о границах на изображении в виде массива `vc::ContourInfo` (см. VideoCaptureLib)
* `Threshold` (unsigned short) - read/write. Изменяя этот атрибут, можно менять порог для поиска границ на изображении во время работы устройства

### States
* `ON` - Состояние по умолчанию
* `FAULT` - Состояние ошибки. Устройство переходит в него, если в properties присутствуют невалидные значения

![image](https://github.com/Karkusha23/my-opencv/assets/16138259/9d78cb09-35ec-4488-aae8-6533fa262c45)

## Сборка и запуск проекта на Windows
1. Установить компилятор С++ v120 от Microsoft, TANGO Controls 9.2.2 (binary) и OpenCV 3.1.0, скачать VideoCaptureLib
2. Сгенерировать в Pogo пустой класс устройства, поставив галочку у VC12 Project
3. Скопировать с заменой файлы .h и .cpp из этой папки в сгенерированную папку проекта
4. Открыть проект в Visual Studio
5. Включить в проекты Class_dll и Class_lib файлы `CamCaptureThread.h` и `CamCaptureThread.cpp`
6. Для кажого проекта в солюшне в его свойствах добавить в каталоги включения `%opencv_root%\build\include` и `%VideoCaptureLib%\include`, и в каталоги библиотек добавить `%opencv_root\build\x64\vc12\lib%`, где `%opencv_root%` - путь, по которому установлен OpenCV

![image](https://github.com/Karkusha23/my-opencv/assets/16138259/6e091d83-9dee-49e9-a34f-60e62fb36583)

7. Для каждого проекта в солюшне добавить строку `opencv_world310d.lib` в дополнительных зависимостях

![image](https://github.com/Karkusha23/my-opencv/assets/16138259/8d45f9c4-54fd-4f70-8ec0-f33ef293f272)

8. Собрать решение
9. В папке с проектом зайти в папку `bin\win64\vc12\debug`, скопировать туда файл `opencv_world300d.dll` из папки `%opencv_root%\build\x64\vc12\bin`, или добавить в переменную среды Path значение `%opencv_root%\build\x64\vc12\bin`
10. Запустить exe, передав в аргументе желаемое имя устройства

