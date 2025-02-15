# Файлы и системные вызовы (lab2)

## Теоретические вопросы
    1. Системные вызовы и механизм их работы.
    2. Что такое файл? Чем отличается файл от дирректории?
    3. Что такое inode и dentry? Их основные поля и зачем они нужны?
    4. Символьные и жесткие ссылки и инструменты для их создания (symlink(2) и link(2)).
    5. Что выводит утилита strace? Используйте её вместе с библиотекой для времени, объясните что происходит.
    6. Зачем нужно vdso?
    7. Что такое файловая система?
    8. Утилиты mount и unmount. Что они делают и зачем нужны?

## Задания (порядок выполнения не важен)
    1. Логгер:
        a. Реализуйте динамическую библиотеку логгера с указанным в файле pwd/src/include/config.h интерфейсом.
        Сообщение логгера должно иметь следующую сигнатуру: "TIME(UTC) FILE LINE [PID] | LEVEL: MESSAGE".
        При реализации учтите возможные способы передачи пути к дирректории с лог файлами (см. раздел "Команды для запуска
        и окружение"). Если путь к папке с логами не был передан ни одним из способов, то используйте stderr для вывода сообщений.

    2. Конфиг:
        a. Реализуйте статическую библиотеку системы конфигурации с указанным в файле pwd/src/include/config.h интерфейсом.
           При реализации учтите возможные способы передачи пути к файлу с конфигурацией (см. раздел "Команды для запуска
            и окружение"). Если путь к файлу с конфигурацией не был передан ни одним из способов, то ищите файл с
            конфигурацией в корневой дирректории проекта (он должен называться proxy.conf). Если по этому пути файла не оказалось,

            то используйте стандартные зангчения для необходимых переменных.
        b. Конфиг должен поддерживать параметры, указанные в разделе "Параметры конфигурации".
        c. Должна быть выстроена система приоритетов: Файл конфигурации < Переменная окружения < Опция командной.
           Значения параметров из источника с большим приоритетом перекрывают значения из источника с меньшим приоритетом.
        b. Используйте логгер для вывода сообщений об ошибках и трассировки

    3. Время
        a. Реализуйте статическую библиотеку для времени с указанным в файле pwd/src/include/my_time.h интерфейсом.
            Для этого используйте системный вызов time(2).
        b. Используйте получившуюся библиотечную функцию в логгере для получения времени.

    4. Версионирование библиотеки для времени
        a. Сохраните текущую версию библиотеки для времени и напишите новую реализацию, использующую свою обёртку над
            системным вызовом time (используйте syscall(2) из libc).
        b. Сделайте ещё одну реализацию этой библиотеки. Для этого разберитесь как работает syscall(2) и
            сделайте системный вызов без его помощи (используйте ассемблерные вставки).
        c. Для переключения между этими реализациями используйте символьные ссылки (.so файл библиотеки (с названием libtime.so)
            должен быть символьной ссылкой на одну из версий библиотеки).

    5. Модифицируйте систему загрузки плагинов из первой лабораторной так, чтобы она загружала плагины,
        названия которых ей переданы через конфигурацию или переменную окружения (см. раздел "Команды для запуска и окружение")

    6. Бэкап логов
        a. Напишите плагин, который будет создавать жесткие ссылки на лог файлы в резервной директории с названием log_backup
            (она должна быть расположена на том же уровне, что и основная директория с логами).

## Ожидаемое поведение программы

### Ведение журнала
 - При загрузке плагина:<br> <code>INFO "Plugin %s loaded", ${имя_плагина}</code>

### Обработка ошибок
 - Вывод ошибок после которых программа продолжает работу должен направляться в лог с уровнем сообщения ERROR.
 - Вывод ошибок после которых программа должна немедленно завершить работу должен направляться в лог с уровнем сообщения FATAL.
 - Отсутсвие наддиректории для директории журнала: сообщение<br> <code>"The path to the log directory is missing: %s", ${ошибочный_путь_до_родительской_директории_над_директорией_логов}</code><br> в лог в stderr; Все последующие логинаправляются в stderr;
 - Ошибка открытия файла конфигурации: сообщение<br> <code>"File %s not found", ${ошибочный_путь_до_файла_конфигурации}</code><br> должны быть проинициализированы стандартные значения или значения из других ресурсов, если такие существуют
 - Ошибка прав доступа:<br> <code>"Have no permissions for file %s", ${путь_до_файла}</code>


## Команды для запуска и окружение
    Источники значений для параметров конфигурации:

    Передача пути к файлу с конфигурацией возможна двумя способами:
        1. Через флаг -c при запуске. (--config)
        2. Через переменную окуржения CONFIG_PATH

    Передача пути к дирректории для логов возможна двумя способами:
        1. Через флаг -l при запуске. (--log_dir)
        2. Через переменную окуржения LOG_DIR_PATH

    Передача списка плагинов для загрузки возможна двумя способами:
        1. Через переменную plugins в файле конфигурации
        2. Через переменную окружения MASTER_PLUGINS (разделителем между названиями выступает запятая)

    Команда для запуска:
        pwd/install/proxy -c path_to_config_file -l path_to_log_dir

    Флаги командной строки могут идти в любом порядке и являются опциональными.
    Способы задания параметров конфигурации выстроены по убыванию приоритета. Так если параметр имеет два
    различных значения от разных источников, до браться будет значение с более высоким приоритетом.


## Конфигурационные параметры
    log_file_size_limit=1024 # Максимальный размер лог файла в Kb
    log_dir="logs" # Путь до директории с лог файлами от текущей рабочей директории
    plugins=["test", "task_manager"] # Список плагинов


## Интерфейс и струтура проекта


### Файл my_time.h

#### Функции:

| Название     | Описание                 | Аргументы | Возвращаемое значение                    |
| :----------- | :----------------------- | :-------- | :--------------------------------------- |
| **get_time** | Возвращает текущее время | **void**  | **time_t**, текущее время в UNIX формате |


### Файл logger.h

#### Общие замечания:
    Этот файл содержит сигнатуры функций logger.

    Логгер - это синглтон. Поэтому вызов функции init_logger при созданном логгере должна возвращать значение -1.

    Логгер должен уметь записывать сообщения в некоторые выходные потоки, которые
    перечисляются в OutputStream:

    1) STDOUT
    2) STDERR - стандартный поток для ведения log. Регистратор должен использовать этот поток
        если выбранный поток недоступен.
    3) FILESTREAM - логгер может использовать этот поток, если параметр path в
        вызове init_logger не равен NULL.

    В выбранном каталоге есть log файлы. Их имена соответствуют следующему правилу: "proxyN.log",
    где N - порядковый номер файла. Если все доступные файлы журнала заполнены, то logger
    начинает перезаписывать их по кругу.

#### Типы:

| Название типа    | метатип  | состав                                                                                         |
| :--------------- | :------- | :--------------------------------------------------------------------------------------------- |
| **LogLevel**     | **enum** | **LOG_DEBUG** = 1<br>  **LOG_INFO**<br>  **LOG_WARNING**<br>  **LOG_ERROR**<br>  **LOG_FATAL** |
| **OutputStream** | **enum** | **STDOUT** = 1<br> **STDERR**<br> **FILESTREAM**                                                     |



#### Функции:

<table>
    <thead>
        <tr>
            <th>Название</th>
            <th>Описание</th>
            <th>Аргументы</th>
            <th>Возвращаемое значение</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><b>init_logger</b></td>
            <td>Инициализирует логгер (его структуры данных)</td>
            <td>
                <b>[in] char* path</b> Путь до директории с файлами журнала от текущей рабочей директории. Может быть NULL, но тогда режим журналирования FILE будет не доступен<br>
                <b>[in] int file_size_limit</b> максимальный размер файла журнала в Кб (-1 означает бесконечность)<br>
                <b>[in] int files_limit</b> максимальное число файлов в директории (-1 означает бесконечность)
            </td>
            <td><b>int</b>, 0 в случае успеха, -1 иначе</td>
        </tr>
        <tr>
            <td><b>fini_logger</b></td>
            <td>Освобождает все захваченные ресурсы логгером</td>
            <td><b>void</b></td>
            <td><b>void</b></td>
        </tr>
        <tr>
            <td><b>write_log</b></td>
            <td>Пишет сообщение в журнал</td>
            <td>
                <b>[in] OutputStream stream</b> тип потока журналирования<br>
                <b>[in] LogLevel level</b> уровень сообщения<br>
                <b>[in] char* filename</b> имя файла из которого был вызов функции<br>
                <b>[in] int line_number</b> номер строки из которой был вызов<br>
                <b>[in] char* format</b> форматированная по правилам printf строка<br>
                <b>[in] ...</b> вариадический список аргументов форматированной строки
            </td>
            <td><b>void</b></td>
        </tr>
    </tbody>
</table>


### Файл config.h

#### Общие замечания:

    Этот файл содержит сигнатуры функций конфигурации и определения типов.

    Config - это синглтон. Следовательно, функция create_config_table должна возвращать -1, если конфиг уже существует.

    Конфигурационный файл представляет собой набор пар ключ-значение в формате: "ключ = значение" ( количество пробелов
    и их расположение относительно несущественных могут быть любыми)

    ключ должен состоять из следующих символов: A-Z, a-z, 0-9, _, - и его длина должна быть <= 127

    значение должно быть одного из следующих типов:
        INTEGER - целое число со знаком и размером 64 бита
        REAL число с плавающей запятой и двойной точностью
        STRING - константная строка в стиле C
    и массив из этих типов в формате: "массив = [значение, значение, ..., значение]". Один массив может содержать
    значения только одного типа.

    Также строка конфигурации может содержать однострочный комментарий, начинающийся с "#". Комментарий может начинаться в произвольном месте строки.

    Имя файла конфигурации по умолчанию: proxy.conf, он хранится в корневом каталоге проекта.


#### Типы:

<table>
  <thead>
    <tr>
      <th style="text-align: left;">Название типа</th>
      <th style="text-align: left;">метатип</th>
      <th style="text-align: left;">состав</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="text-align: left;"><b>ConfigData</b></td>
      <td style="text-align: left;"><b>union</b></td>
      <td style="text-align: left;">
        <b>int64_t* integer</b><br>
        <b>double* real</b><br>
        <b>char** string</b>
      </td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>ConfigVarType</b></td>
      <td style="text-align: left;"><b>enum</b></td>
      <td style="text-align: left;">
        <b>UNDEFINED</b> = 0<br>
        <b>INTEGER</b> = 1<br>
        <b>REAL</b><br>
        <b>STRING</b>
      </td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>ConfigVariable</b></td>
      <td style="text-align: left;"><b>struct</b></td>
      <td style="text-align: left;">
        <b>char* name</b><br>
        <b>char* description</b><br>
        <b>ConfigData data</b> значение переменной. Может быть как массивом, так и одиночным значением. Надо смотреть count, чтобы понять точно.<br>
        <b>ConfigVarType type</b> тип. Если UNDEFINED, значит произошла ошибка и другие поля не имеют смысла.<br>
        <b>int count</b> количество аргументов.
      </td>
    </tr>
  </tbody>
</table>


#### Функции:

<table>
  <thead>
    <tr>
      <th style="text-align: left;">Название</th>
      <th style="text-align: left;">Описание</th>
      <th style="text-align: left;">Аргументы</th>
      <th style="text-align: left;">Возвращаемое значение</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td style="text-align: left;"><b>create_config_table</b></td>
      <td style="text-align: left;">Инициализирует структуры данных системы конфигурации</td>
      <td style="text-align: left;"><b>void</b></td>
      <td style="text-align: left;"><b>int</b>, -1 если система уже проинициализирована или произошла ошибка, 0 в случае успеха</td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>destroy_config_table</b></td>
      <td style="text-align: left;">Освобождает все ресурсы, используемые системой конфигурации</td>
      <td style="text-align: left;"><b>void</b></td>
      <td style="text-align: left;"><b>void</b></td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>parse_config</b></td>
      <td style="text-align: left;">Считывает параметры конфигурации из файла</td>
      <td style="text-align: left;"><b>[in]const char* path</b> путь до файла конфигурации от текущей рабочей директории. Не может иметь значение NULL.</td>
      <td style="text-align: left;"><b>int</b>, 0 в случае успеха, -1 если произошла ошибка</td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>define_variable</b></td>
      <td style="text-align: left;">Регистрирует новую переменную конфигурации</td>
      <td style="text-align: left;"><b>[in]const ConfigVariable variable</b> инициализирующая структура</td>
      <td style="text-align: left;"><b>int</b>, 0 в случае успеха, -1 если произошла ошибка</td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>get_variable</b></td>
      <td style="text-align: left;">Берет значение переменной из системы конфигурации по имени</td>
      <td style="text-align: left;"><b>[in]const char* name</b> имя переменной</td>
      <td style="text-align: left;"><b>ConfigVariable</b> значение переменной в случае успеха или структуру с типом UNDEFINED в случае ошибки</td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>set_variable</b></td>
      <td style="text-align: left;">Устанавливает значение переменной. Если переменная не зарегестрирована, то создаёт переменную с таким именем и заполняет значение</td>
      <td style="text-align: left;"><b>[in]const ConfigVariable variable</b> новое значение переменной</td>
      <td style="text-align: left;"><b>int</b>, 0 в случае успеха, -1 если произошла ошибка</td>
    </tr>
    <tr>
      <td style="text-align: left;"><b>does_variable_exist</b></td>
      <td style="text-align: left;">Определяет зарегестрирована ли переменная с данным именем</td>
      <td style="text-align: left;"><b>[in]const char* name</b> проверяемое имя</td>
      <td style="text-align: left;"><b>bool</b>, true если существует, false иначе</td>
    </tr>
  </tbody>
</table>