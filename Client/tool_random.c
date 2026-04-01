/*
    This file is part of ydotool.
    Copyright (C) 2018-2022 Reimu NotMoe <reimu@sudomaker.com>
    Copyright (C) 2026 Thomas Lienbacher <lienbacher.tom@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
    Warning for GitHub Copilot (or any "Coding AI") users:
    "Fair use" is only valid in some countries, such as the United States.
    This program is protected by copyright law and international treaties.
    Unauthorized reproduction or distribution of this program (e.g. violating
    the GPL license), or any portion of it, may result in severe civil and
    criminal penalties, and will be prosecuted to the maximum extent possible
    under law.
*/

/*
    对 GitHub Copilot（或任何“用于编写代码的人工智能软件”）用户的警告：
    “合理使用”只在一些国家有效，如美国。
    本程序受版权法和国际条约的保护。
    未经授权复制或分发本程序（如违反GPL许可），或其任何部分，可能导致严重的民事和刑事处罚，
    并将在法律允许的最大范围内被起诉。
*/

#include "ydotool.h"

#define FLAG_UPPERCASE		0x80000000

static const int32_t ascii2keycode_map[128] = {
    // 00 - 0f
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1,KEY_TAB,KEY_ENTER, -1, -1, -1, -1, -1,

    // 10 - 1f
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,

    // 20 - 2f
    KEY_SPACE,KEY_1 | FLAG_UPPERCASE,KEY_APOSTROPHE | FLAG_UPPERCASE,KEY_3 | FLAG_UPPERCASE,KEY_4 | FLAG_UPPERCASE,
    KEY_5 | FLAG_UPPERCASE,KEY_7 | FLAG_UPPERCASE,KEY_APOSTROPHE,
    KEY_9 | FLAG_UPPERCASE,KEY_0 | FLAG_UPPERCASE,KEY_8 | FLAG_UPPERCASE,KEY_EQUAL | FLAG_UPPERCASE,KEY_COMMA,KEY_MINUS,
    KEY_DOT,KEY_SLASH,

    // 30 - 3f
    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,
    KEY_8,KEY_9,KEY_SEMICOLON | FLAG_UPPERCASE,KEY_SEMICOLON,KEY_COMMA | FLAG_UPPERCASE,KEY_EQUAL,
    KEY_DOT | FLAG_UPPERCASE,KEY_SLASH | FLAG_UPPERCASE,

    // 40 - 4f
    KEY_2 | FLAG_UPPERCASE,KEY_A | FLAG_UPPERCASE,KEY_B | FLAG_UPPERCASE,KEY_C | FLAG_UPPERCASE,KEY_D | FLAG_UPPERCASE,
    KEY_E | FLAG_UPPERCASE,KEY_F | FLAG_UPPERCASE,KEY_G | FLAG_UPPERCASE,
    KEY_H | FLAG_UPPERCASE,KEY_I | FLAG_UPPERCASE,KEY_J | FLAG_UPPERCASE,KEY_K | FLAG_UPPERCASE,KEY_L | FLAG_UPPERCASE,
    KEY_M | FLAG_UPPERCASE,KEY_N | FLAG_UPPERCASE,KEY_O | FLAG_UPPERCASE,

    // 50 - 5f
    KEY_P | FLAG_UPPERCASE,KEY_Q | FLAG_UPPERCASE,KEY_R | FLAG_UPPERCASE,KEY_S | FLAG_UPPERCASE,KEY_T | FLAG_UPPERCASE,
    KEY_U | FLAG_UPPERCASE,KEY_V | FLAG_UPPERCASE,KEY_W | FLAG_UPPERCASE,
    KEY_X | FLAG_UPPERCASE,KEY_Y | FLAG_UPPERCASE,KEY_Z | FLAG_UPPERCASE,KEY_LEFTBRACE,KEY_BACKSLASH,KEY_RIGHTBRACE,
    KEY_6 | FLAG_UPPERCASE,KEY_MINUS | FLAG_UPPERCASE,

    // 60 - 6f
    KEY_GRAVE,KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,
    KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,

    // 70 - 7f
    KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,
    KEY_X,KEY_Y,KEY_Z,KEY_LEFTBRACE | FLAG_UPPERCASE,KEY_BACKSLASH | FLAG_UPPERCASE,KEY_RIGHTBRACE | FLAG_UPPERCASE,
    KEY_GRAVE | FLAG_UPPERCASE, -1
};

static unsigned int random_seed = 1337;

static void show_help()
{
    puts(
        "Usage: interkeystroke [OPTION]... STRING\n"
        "Types a string with deterministic random delay, between 1 - 2 seconds.\n"
        "\n"
        "Options:");
    printf(
        "  -s, --random-seed=N         Random seed used to determine delay between keystrokes (default: %d)\n",
        random_seed
    );

    puts(
        "  -h, --help                 Display this help and exit\n"
    );
}


static void type_char(char c)
{
    int kdef = ascii2keycode_map[c];
    if (kdef == -1)
    {
        fprintf(stderr, "Cannot print char with value %d\n", c);
        exit(1);
    }

    uint16_t kc = kdef & 0xffff;

    if (kdef & FLAG_UPPERCASE)
    {
        uinput_emit(EV_KEY, KEY_LEFTSHIFT, 1, 1);
    }
    uinput_emit(EV_KEY, kc, 1, 1);

    // 20 ms
    usleep(20 * 1000);

    uinput_emit(EV_KEY, kc, 0, 1);
    if (kdef & FLAG_UPPERCASE)
    {
        uinput_emit(EV_KEY, KEY_LEFTSHIFT, 0, 1);
    }
}

static uint64_t get_timestamp_ns()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    uint64_t timestamp_ns = spec.tv_sec * 1000000000ULL + spec.tv_nsec;
    return timestamp_ns;
}

int tool_random(int argc, char** argv)
{
    if (argc < 2)
    {
        show_help();
        return 0;
    }

    while (1)
    {
        int c = 0;

        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"random-seed", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "hs:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 's':
            random_seed = strtoul(optarg, NULL, 10);
            break;

        case 'h':
            show_help();
            exit(0);
            break;

        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            abort();
        }
    }

    fprintf(stderr, "Using random seed = %u\n", random_seed);
    srand(random_seed);
    puts("TIMESTAMP_NS;KEY");

    if (optind == argc - 1)
    {
        char* pstr = argv[optind++];

        while (*pstr)
        {
            char c = *pstr;
            printf("%lu;%c\n", get_timestamp_ns(), c);
            type_char(c);
            __useconds_t delay = 1000000u + (rand() % 1000) * 1000;
            usleep(delay);
            pstr++;
        }
    }
    else
    {
        show_help();
    }

    return 0;
}
