Собрано и написано на Ubuntu 22.04

Здесь описано как код работает и как собирать проект
Эта часть будет поделена на несколько частей:

1) Как собирать проект и как запустить
2) Пояснение того, как работает код

Нужно создать папку build, где вводим эти команды:
cmake ..
make

После сборки, можем прогнать тесты
./runTests

Можем запустить наш сервер
./server port maxConnection

Пример запуска
./server 8080 5 

Можем запустить клиента
./client hostname port

Пример запуска
./client localhost 8080

Чтобы завершить работу сервера или клиента нужно нажать сочетание клавиш Ctrl + C

2)

Данный проект реализует TCP-сервер для хранения данных в формате ключ-значение с поддержкой многопоточности и логированием. Сервер принимает команды от клиента и выполняет их

server.h
Заголовочный файл для класса Server, который реализует TCP-сервер для хранения данных в формате ключ-значение.

server.cpp
Реализация методов класса Server. В этом файле реализуются все основные функции сервера, такие как запуск, остановка, обработка команд.

common.h
Заголовочный файл, содержащий общие определения и функции для обработки ошибок.

main.cpp
Основной файл для запуска сервера. Парсит аргументы командной строки для получения порта и запускает сервер.

client.cpp
Файл, реализующий TCP-клиент для взаимодействия с сервером. Клиент позволяет пользователю вводить команды и отправлять их на сервер.

test.cpp
Файл с юнит-тестами для сервера, использующий библиотеку Google Test. Тесты проверяют основные функции сервера, такие как добавление, получение, удаление и подсчет ключей.

