//ДУРАК  !!!9 + !!!
//Карты отрисовывать.На карте есть номинал и масть.В зависимости от масти карты соответствующего цвета - красного или чёрного.
//В левом верхнем углу отрисовывается оставшаяся после раздачи колода.
//На "рубашке" верхней карты отображается количество оставшихся в колоде карт.Рядом показана карта козырной масти.
//Снизу отображаются карты из "руки" пользователя.В верхней части окна - отображаются карты бота.Карты отображаются "рубашками" вверх.
//Первым осуществляет ход тот, у кого есть козырная карта меньшего номинала.Карту нужно "предъявить" перед первым ходом. (Это происходит автоматически и для бота и для пользователя)
//Перемешение между картами пользователя осуществляется стрелочками.Выбираемая карты выдвигается вверх из общего ряда.Подтверждение выбора - нажатие Enter.
//Если происходит попытка отбиться - нужно проверять карту на соответствие моменту.Т.е.чтобы выбранной картой можно было отбить карту, лежащую на "столе" или подбросить, если отбивается бот.

//v 1_01
//Исправлено: 
//1. Глюк при доборе карт, когда игроки не могли взять все карты из колоды, добор по очереди последних карт колоды пока корректно не реализован
//2. Глюк, когда у игрока или бота было меньше 6 карт, а колода добора пуста функция BITO не срабатывала 
//Добавлено:
//1.Функция ограничивающая поле ввода символов и выводящая * при вводе пароля

//v 1_02
// Исправлено:
//1. Исправлена работа функции добора - игроки берут карты поровну вслучае если карт в колоде меньше чем нужно игрокам.
//2. Исправлена ошибка, когда бот отбился - на столе 12 карт, а игрок мог ему подкидывать карты (правильность работы не проверена)
//3. Исправлена ошибка, когда игрок отбился - на столе 12 карт, а бот мог ему подкидывать карты (правильность работы не проверена)
//4. Исправдена ошибка пересчета шага координат для игрока, передовался шаг координат карт бота.
// Добавлено:
//1. Отображаются символы кириллицы
//2. Отцентрировано стартовое меню
//3. Добавлен костыль с зарисовкой крайней правой нижней ячейки через  SetPixel 
//4. Добавлено контрольное обнуление массивов структур NullPoolCards (есть подозрение, что из-за этого при повторных матчах возникали глюки с добром тк. в массивах структур содержался мусор)
//5. Реализован подброс карт игроком и ботом

//v 1_03
// Исправлено:
//1. Исправлена ошибка с генерацией карт - переменные условий не обнулялись

#include <iostream>
#include <string.h>
//#include <stdio.h>
//#include <time.h>
#include <Windows.h>
#include <conio.h>
//#include <algorithm>
#include <io.h>

#include <fstream> // запись/чтение в файл
#include <sstream>  // Требуется для работы функций toString и fromString

// Предварительные настройки игры

int sleep1000 = 1000;
int sleep300 = 300;
int sleep175 = 175;
int sleep75 = 75;
bool intro = 1;

// Вероятность подбрасывания ботом карт в начале игры
int chance_to_drop = 85; //67-85%

// Только для презентации
bool presentation_pause = 0;

using namespace std;
HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
enum Dvoynie_ramki
{
	LeftUpper = 201, RightUpper = 187, LeftDown = 200, RightDown = 188, Gorizont = 205, Vertikal = 186
};
enum Odinarnie_ramki
{
	One_LeftUpper = 218, One_RightUpper = 191, One_LeftDown = 192, One_RightDown = 217, One_Gorizont = 196, One_Vertikal = 124
};
enum Color
{
	Black, Blue, Green, Cyan, Red, Magenta, Brown,
	LightGray, DarkGray, LightBlue, LightGreen, LightCyan, LightRed, LightMagenta, Yellow, White
};
void SetColor(Color text, Color background)
{
	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}
void SetCursor(int x, int y)
{
	COORD myCoords = { x,y };//инициализируем передаваемыми значениями объект координат
	SetConsoleCursorPosition(hStdOut, myCoords);
}
enum Direction { Up = 72, Left = 75, Right = 77, Down = 80, Enter = 13, esc = 27 };
void setCursor(int x, int y)
{
	COORD myCoords = { x,y };//инициализируем передаваемыми значениями объект координат
	SetConsoleCursorPosition(hStdOut, myCoords);
}

//Чистка памяти от данных двумерного динамического массива
template <typename Tarr>
void Delete_D_Arr(Tarr** arr, int row)
{
	if (arr != nullptr)
	{
		for (size_t i = 0; i < row; i++)
		{
			delete[] arr[i];
		}
		delete[] arr;
	}
}

// Колода карт

struct  Koloda
{
	// Номинал
	int number;
	// Масть
	char suit;
	//название карты
	string name;
	// Козырь
	bool trump;
};

// Русификация
// Способ проще: WinAPI system("chcp 1251>NUL"); system("chcp 866>NUL"); НО! Побочка - если записывается массив - комп очень долго думает Правильно без файла!: system("chcp 1251 > NUL"); system("chcp 866 > NUL");
// < -это перенаправление ввода программы, так же как > -перенаправление стандартного вывода программы.NUL - это устройство(псевдофайл), точнее пустое устройство.Вывод в него не даёт ничего, а ввод эквивалентен чтению пустого файла.

//void RUCIN(char magic[])

string RUCIN(string input)
{
	char magic[256];
	// c_str() формирует массив строк в стиле си
	strcpy_s(magic, input.c_str());

	int delta = 256;
	for (size_t i = 0; magic[i] != '\0'; i++)
	{
		// Ё
		if ((int)magic[i] + delta == 168)
		{
			magic[i] += 72;
		}
		// ё
		else if ((int)magic[i] + delta == 184)
		{
			magic[i] += 57;
		}
		//А-я
		else if ((int)magic[i] + delta >= 192 and (int)magic[i] + delta <= 239)
		{
			magic[i] -= 64;
		}
		else if ((int)magic[i] + delta >= 240 and (int)magic[i] + delta <= 255)
		{
			magic[i] -= 16;
		}
	}
	return magic;
}


//Настройки пользователей/игроков

//Функция ввода логина или пароля
string Login_Password(char lp = 'L')
{
	char Login[18];
	int i = 0;
	while (true)
	{
		if (i < 17)
		{
			Login[i] = _getch();
			if (Login[i] == 32 or Login[i] == 9)
			{
				Login[i] = (char)'_';
			}
			if (Login[i] == '\r')
			{
				break;
			}
			else if (Login[i] == '\b')
			{
				if (i > 0)
				{
					i--;
					cout << "\b \b";
				}
			}
			else
			{
				if (lp == 'L')
				{
					cout << Login[i];
				}
				else if (lp == 'P')
				{
					cout << "*";
				}
				i++;
			}
		}
		else
		{
			Login[i] = _getch();
			if (Login[i] == '\b')
			{
				if (i > 0)
				{
					i--;
					cout << "\b \b";
				}
			}
			else if (Login[i] == '\r')
			{
				break;
			}
		}
	}
	Login[i] = '\0';
	return Login;
}

int maxplayerslot = 12;
struct Gamers
{
	int user_id = 0;
	string name;
	string password;
	int games = 0;
	int wins = 0;
};

// Запись данных в  строку структуры GamersTable
void AddDataToGamersTableRow(Gamers& GamersTable, string username, string userpassword, int user_id = 0, int games = 0, int wins = 0)
{
	GamersTable.user_id = user_id;
	GamersTable.name = username;
	GamersTable.password = userpassword;
	GamersTable.games = games;
	GamersTable.wins = wins;
}

// Запись в конкретную строку таблицы структуры
void AddDataToGamersTableRowZ(Gamers* GamersTable, string username, string userpassword, int user_id = 0, int games = 0, int wins = 0)
{
	GamersTable[user_id].user_id = user_id;
	GamersTable[user_id].name = username;
	GamersTable[user_id].password = userpassword;
	GamersTable[user_id].games = games;
	GamersTable[user_id].wins = wins;
}



// Запись данных в таблицу структуры GamersTable
void AddDataToGamersTable(Gamers* PlayersTable, string& username, string& userpassword, int user_id = 0, int games = 0, int wins = 0, int player_number = 0, bool add = true)
{
	if (add)
	{
		player_number++;
	}
	for (size_t user_id = 0; user_id < player_number; user_id++)
	{
		AddDataToGamersTableRow(PlayersTable[user_id], username, userpassword, user_id, games, wins);
	}

}


//
//БЛОК РАБОТЫ С ФАЙЛАМИ НАЧАЛО
//

int number_columns_in_file = 5;
// переменная для функции подсчета строк
string temp;

//Функция подсчета строки из файла
int FuncReadF_row_number(string str)
{
	int col = 0;
	//Открытие потока для чтения из файла
	ifstream from_file("PlayersSuperSecureData.txt");
	if (from_file.is_open())
	{


		while (getline(from_file, str))
		{
			col++;
		}
	}
	else
	{
		cout << "file not read" << endl;
	}
	from_file.close();
	return col;
}

//Функция записи значений в файл
void AddGamersTableRowDataToFile(int player_id, string player_name, string player_password, int games, int wins)
{
	ofstream to_file("PlayersSuperSecureData.txt", ios::app); // ::app - добавление в конец файла
	if (to_file.is_open())
	{
		if (player_id != -1)
		{
			to_file << player_id << '\t' << player_name << '\t' << player_password << '\t' << games << '\t' << wins << endl;
		}
	}
	else
	{
		cout << "file not read" << endl;
	}
	to_file.close();
}

// https://ru.stackoverflow.com/questions/667229/%D0%97%D0%B0%D0%BF%D0%B8%D1%81%D1%8C-%D0%B4%D0%B0%D0%BD%D0%BD%D1%8B%D1%85-%D0%B8%D0%B7-%D1%84%D0%B0%D0%B9%D0%BB%D0%B0-%D0%B2-%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%82%D1%83%D1%80%D1%83-%D1%81
// istream -интерфейс для чтения данных из потока, ifstream - производный класс
istream& operator>> (istream& is, Gamers& gt)
{
	is >> gt.user_id >> gt.name >> gt.password >> gt.games >> gt.wins;
	return is;
}

//Функция чтения данных из файла и их записи в структуру GamersTable
void ReadFileToGamersTableRow(Gamers& GamersTable, int count)
{

	ifstream from_file("PlayersSuperSecureData.txt", ios::binary);
	if (from_file.is_open())
	{
		for (size_t i = 0; i < count; i++)
		{
			from_file.ignore(256, '\n');
		}
		//Работает только с is
		from_file >> GamersTable;
	}
	else
	{
		cout << "file not read" << endl;
	}
	from_file.close();
}
//Функция чтения данных из файла и их записи в таблицу структуры GamersTable
void ReadFileToGamersTable(Gamers* GamersTable, int number_row)
{
	ifstream from_file("PlayersSuperSecureData.txt", ios::binary);

	for (size_t i = 0; i < number_row; i++)
	{
		ReadFileToGamersTableRow(GamersTable[i], i);
	}
	from_file.close();
}

// Функция для перезаписи файла с учетом удаленной позиции
void ReWriteFileGamersTable(Gamers* GamersTable, int size, int rkey)
{
	ofstream to_file("PlayersSuperSecureData.txt", ios::out); // ::out - поток открывается для записи в файл, старые данные в файле удаляются
	if (to_file.is_open())
	{
		int count_slot = 0;
		for (size_t i = 0; i < size; i++)
		{

			if (i != rkey)
			{
				to_file << count_slot << '\t' << GamersTable[i].name << '\t' << GamersTable[i].password << '\t' << GamersTable[i].games << '\t' << GamersTable[i].wins << endl;
				count_slot++;
			}
		}
	}
	else
	{
		cout << "file not read" << endl;
	}
	to_file.close();
}

// Функция перезаписи файла с учетом изменений Games and Wins
void ReWriteFileGamersTable(Gamers* GamersTable, int size)
{
	ofstream to_file("PlayersSuperSecureData.txt", ios::out); // ::out - поток открывается для записи в файл, старые данные в файле удаляются
	if (to_file.is_open())
	{
		int count_slot = 0;
		for (size_t i = 0; i < size; i++)
		{
			to_file << count_slot << '\t' << GamersTable[i].name << '\t' << GamersTable[i].password << '\t' << GamersTable[i].games << '\t' << GamersTable[i].wins << endl;
			count_slot++;
		}
	}
	else
	{
		cout << "file not read" << endl;
	}
	to_file.close();
}

//
//БЛОК РАБОТЫ С ФАЙЛАМИ КОНЕЦ
//


//
// БЛОК ОТРИСОВКИ ФОНА
// 
//"Обнуление" фона
void Nullfone(char** fone, int row, int col)
{
	int count = 3;
	system("cls");
	for (size_t i = 0; i < row; i++)
	{
		for (size_t j = 0; j < col; j++)
		{
			if (i == row - 1 or j == 0 or i == 0 or j == col - 1)
			{
				fone[i][j] = count;
				count++;
				if (count == 7)
				{
					count = 3;
				}
			}
			else
			{
				fone[i][j] = ' ';
			}
		}
	}
}
//Отрисовка фона(только игровой стол)
void PrintFon(char** fone, int row, int col)
{
	int controlj = col - 1;
	for (size_t i = 0; i < row; i++)
	{
		for (size_t j = 0; j < col; j++)
		{
			if (j == controlj and i == row - 1)
			{
				break;
			}
			else
			{
				if (fone[i][j] >= 3 and fone[i][j] <= 4)
				{
					SetColor(Red, Brown);
				}
				else if (fone[i][j] >= 5 and fone[i][j] <= 6)
				{
					SetColor(DarkGray, Brown);
				}
				else
				{
					SetColor(White, Green);
				}
				cout << (char)fone[i][j];
				SetColor(White, Black);
			}
		}
	}
	// Костыль с бубнухой в правом нижнем углу
	HDC hdc = GetDC(GetConsoleWindow());
	int x = 1433, delta_x = x + 7;
	int y = 944, delta_y = y + 16;
	int dx = (x + delta_x) / 2;
	int dy = (y + delta_y) / 2;
	int dxx = (x + delta_x) / 2;

	for (size_t i = y; i < delta_y; i++)
	{
		if (i >= y and i < y + 4)
		{
			for (size_t j = x; j < delta_x; j++)
			{
				SetPixel(hdc, j, i, RGB(200, 150, 32));
			}

		}
		else if (i < y + 8)
		{
			for (size_t j = x; j < delta_x; j++)
			{
				if (j < dx or j > dxx)
				{
					SetPixel(hdc, j, i, RGB(200, 150, 32));

				}
				else
				{
					SetPixel(hdc, j, i, RGB(185, 0, 0));

				}
			}
			dx--;
			dxx++;
		}
		else /*if (i > y+4)*/
		{
			for (size_t j = x; j < delta_x; j++)
			{
				if (j < dx or j > dxx)
				{
					SetPixel(hdc, j, i, RGB(200, 150, 32));
				}
				else
				{
					SetPixel(hdc, j, i, RGB(185, 0, 0));
				}
			}
			dx++;
			dxx--;
		}
	}
}

//
//БЛОК ОТРИСОВКИ МАССИВОВ
//
// Отрисовка массива табло
void FillCurrentFone(char** currentfone, int row, int col)
{
	for (size_t i = 0; i < row; i++)
	{
		for (size_t j = 0; j < col; j++)
		{
			if (i == 0 and j == 0)
			{
				currentfone[i][j] = LeftUpper;
			}
			else if (i == row - 1 and j == col - 1)
			{
				currentfone[i][j] = RightDown;
			}
			else if (i == 0 and j == col - 1)
			{
				currentfone[i][j] = RightUpper;
			}
			else if (i == row - 1 and j == 0)
			{
				currentfone[i][j] = LeftDown;
			}
			else if ((i == 0 or i == row - 1) and (j >= 1 and j < col - 1))
			{
				currentfone[i][j] = Gorizont;
			}
			else if ((i >= 1 and i < row - 1) and (j == 0 or j == col - 1))
			{
				currentfone[i][j] = Vertikal;
			}
			else
			{
				currentfone[i][j] = ' ';
			}
		}
	}
}
void PrintCurrentFone(char** arrfone, int row, int col, Color color_1, Color color_2, int X = 0, int Y = 0, bool shadow = true)
{
	FillCurrentFone(arrfone, row, col);
	//тень
	if (shadow == true)
	{
		for (size_t i = 0; i < row; i++)
		{

			for (size_t j = 0; j < col; j++)
			{
				if (i == row - 1)
				{
					SetCursor(X + 1 + j, Y + row);
					SetColor(Black, Black);
					cout << (char)arrfone[i][j];
					SetColor(White, Black);
				}
				if (j == col - 1)
				{
					SetCursor(X + col, Y + 1 + i);
					SetColor(Black, Black);
					cout << (char)arrfone[i][j];
					SetColor(White, Black);
				}
			}
		}
	}
	//основнеое меню
	for (size_t i = 0; i < row; i++)
	{
		SetCursor(X, Y + i);
		for (size_t j = 0; j < col; j++)
		{

			SetColor(color_1, color_2);
			cout << (char)arrfone[i][j];
			SetColor(White, Black);

		}
	}
}

// Закраска определенным цветом и размеров в указанных координатах
void PrintCurrentFone(char** arrfone, int row, int col, Color color, int X = 0, int Y = 0)
{
	for (size_t i = 0; i < row; i++)
	{
		for (size_t j = 0; j < col; j++)
		{
			arrfone[i][j] = ' ';
		}
	}
	for (size_t i = 0; i < row; i++)
	{
		SetCursor(X, Y + i);
		for (size_t j = 0; j < col; j++)
		{

			SetColor(color, color);
			cout << (char)arrfone[i][j];
		}
		SetColor(White, Black);
	}
}

// Печать таблицы игроков
void ShowGamersTable(Gamers* GamersTable, int size, Color color_1, Color color_2, int X = 0, int Y = 0, int dist_1 = 0, int dist_2 = 0, int dist_3 = 0, int dist_4 = 0)
{
	SetColor(color_1, color_2);
	for (size_t i = 0; i < size; i++)
	{
		SetCursor(X, Y + i);
		cout << GamersTable[i].user_id + 1;
		SetCursor(X + dist_1, Y + i);
		cout << GamersTable[i].name;
		SetCursor(X + dist_2, Y + i);
		cout << GamersTable[i].games;
		SetCursor(X + dist_3, Y + i);
		cout << GamersTable[i].wins;
	}
	SetColor(White, Black);
}

//Функция отрисовки карты
void PrintCard(Koloda& Card, char** card_picture, int X, int Y, int row, int col, Color color_1, Color color_2, bool hidden = false)
{
	// Заполнение рубашки
	if (hidden)
	{
		for (size_t i = 0; i < row; i++)
		{
			for (size_t j = 0; j < col; j++)
			{

				if (i == 0 and j == 0)
				{
					card_picture[i][j] = One_LeftUpper;
				}
				else if (i == row - 1 and j == col - 1)
				{
					card_picture[i][j] = One_RightDown;
				}
				else if (i == 0 and j == col - 1)
				{
					card_picture[i][j] = One_RightUpper;
				}
				else if (i == row - 1 and j == 0)
				{
					card_picture[i][j] = One_LeftDown;
				}
				else if ((i == 0 or i == row - 1) and (j >= 1 and j < col - 1))
				{
					card_picture[i][j] = One_Gorizont;
				}
				else if ((i >= 1 and i < row - 1) and (j == 0 or j == col - 1))
				{
					card_picture[i][j] = One_Vertikal;
				}
				else
				{
					if (i % 2 == 0 and j % 2 != 0)
					{
						card_picture[i][j] = 178;
					}
					if (i % 2 != 0 and j % 2 == 0)
					{
						card_picture[i][j] = 178;
					}
					if (i % 2 == 0 and j % 2 == 0)
					{
						card_picture[i][j] = 177;
					}
					if (i % 2 != 0 and j % 2 != 0)
					{
						card_picture[i][j] = 177;
					}
				}
			}
		}
	}
	// Заполнение карты (если hidden == false)
	else
	{
		for (size_t i = 0; i < row; i++)
		{
			for (size_t j = 0; j < col; j++)
			{
				card_picture[i][j] = ' ';
			}
		}
		//Шестерка
		if (Card.number == 0)
		{
			card_picture[0][0] = Card.number + 54;
			card_picture[8][8] = Card.number + 54;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[1][2] = Card.suit;
			card_picture[4][2] = Card.suit;
			card_picture[7][2] = Card.suit;
			card_picture[1][6] = Card.suit;
			card_picture[4][6] = Card.suit;
			card_picture[7][6] = Card.suit;
		}
		// Семерка
		if (Card.number == 1)
		{
			card_picture[0][0] = Card.number + 54;
			card_picture[8][8] = Card.number + 54;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[1][2] = Card.suit;
			card_picture[4][2] = Card.suit;
			card_picture[7][2] = Card.suit;
			card_picture[1][6] = Card.suit;
			card_picture[4][6] = Card.suit;
			card_picture[7][6] = Card.suit;
			card_picture[2][4] = Card.suit;
		}
		//Восьмерка
		if (Card.number == 2)
		{
			card_picture[0][0] = Card.number + 54;
			card_picture[8][8] = Card.number + 54;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[1][2] = Card.suit;
			card_picture[4][2] = Card.suit;
			card_picture[7][2] = Card.suit;
			card_picture[1][6] = Card.suit;
			card_picture[4][6] = Card.suit;
			card_picture[7][6] = Card.suit;
			card_picture[2][4] = Card.suit;
			card_picture[6][4] = Card.suit;
		}
		//Девятка
		if (Card.number == 3)
		{
			card_picture[0][0] = Card.number + 54;
			card_picture[8][8] = Card.number + 54;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[1][2] = Card.suit;
			card_picture[3][2] = Card.suit;
			card_picture[5][2] = Card.suit;
			card_picture[7][2] = Card.suit;
			card_picture[1][6] = Card.suit;
			card_picture[3][6] = Card.suit;
			card_picture[5][6] = Card.suit;
			card_picture[7][6] = Card.suit;
			card_picture[4][4] = Card.suit;
		}
		//Десятка
		if (Card.number == 4)
		{
			card_picture[0][0] = Card.number + 45;
			card_picture[0][1] = Card.number + 44;
			card_picture[8][7] = Card.number + 45;
			card_picture[8][8] = Card.number + 44;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[1][2] = Card.suit;
			card_picture[3][2] = Card.suit;
			card_picture[5][2] = Card.suit;
			card_picture[7][2] = Card.suit;
			card_picture[1][6] = Card.suit;
			card_picture[3][6] = Card.suit;
			card_picture[5][6] = Card.suit;
			card_picture[7][6] = Card.suit;
			card_picture[2][4] = Card.suit;
			card_picture[6][4] = Card.suit;
		}
		//Валет
		if (Card.number == 5)
		{
			card_picture[0][0] = Card.number + 69;
			card_picture[8][8] = Card.number + 69;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[2][4] = Card.number + 69;
			card_picture[3][4] = Card.number + 60;
			card_picture[4][4] = Card.number + 62;
			card_picture[5][4] = Card.number + 70;
			card_picture[7][4] = Card.number + 6;
		}
		//Дама
		if (Card.number == 6)
		{
			card_picture[0][0] = Card.number + 75;
			card_picture[8][8] = Card.number + 75;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[2][4] = Card.number + 75;
			card_picture[3][4] = Card.number + 79;
			card_picture[4][4] = Card.number + 63;
			card_picture[5][4] = Card.number + 63;
			card_picture[6][4] = Card.number + 72;
		}
		//Король
		if (Card.number == 7)
		{
			card_picture[0][0] = Card.number + 68;
			card_picture[8][8] = Card.number + 68;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[2][4] = Card.number + 68;
			card_picture[3][4] = Card.number + 66;
			card_picture[4][4] = Card.number + 71;
			card_picture[5][4] = Card.number + 64;
			card_picture[7][4] = Card.number + 4;
		}
		//Туз
		if (Card.number == 8)
		{
			card_picture[0][0] = Card.number + 57;
			card_picture[8][8] = Card.number + 57;
			card_picture[1][0] = Card.suit;
			card_picture[7][8] = Card.suit;
			card_picture[3][4] = Card.number + 57;
			card_picture[4][4] = Card.number + 59;
			card_picture[5][4] = Card.number + 61;
		}
	}
	// Отрисовка рубашки карты
	if (hidden)
	{
		SetColor(color_1, color_2);
		SetCursor(X, Y);
		for (size_t i = 0; i < row; i++)
		{
			SetCursor(X, Y + i);
			for (size_t j = 0; j < col; j++)
			{
				cout << card_picture[i][j];
			}
			cout << endl;
		}
		SetColor(White, Black);
	}
	// Отрисовка карты
	else
	{
		if (Card.suit == 3 or Card.suit == 4)
		{
			SetColor(Red, White);
		}
		else
		{
			SetColor(Black, White);
		}
		SetCursor(X, Y);
		for (size_t i = 0; i < row; i++)
		{
			SetCursor(X, Y + i);
			for (size_t j = 0; j < col; j++)
			{
				cout << card_picture[i][j];
			}
			cout << endl;
		}
		SetColor(White, Black);
	}
}

// Функция отрисовки горизонтальной рубашки
void PrintSuitGorizont(int X, int Y, Color color_1, Color color_2)
{
	int card_height = 5;
	int card_width = 19;
	char** gor_card = new char* [card_height];
	for (size_t i = 0; i < card_height; i++)
	{
		gor_card[i] = new char[card_width];
	}
	for (size_t i = 0; i < card_height; i++)
	{
		for (size_t j = 0; j < card_width; j++)
		{
			gor_card[i][j] = ' ';
		}
	}

	for (size_t i = 0; i < card_height; i++)
	{
		for (size_t j = 0; j < card_width; j++)
		{

			if (i == 0 and j == 0)
			{
				gor_card[i][j] = One_LeftUpper;
			}
			else if (i == card_height - 1 and j == card_width - 1)
			{
				gor_card[i][j] = One_RightDown;
			}
			else if (i == 0 and j == card_width - 1)
			{
				gor_card[i][j] = One_RightUpper;
			}
			else if (i == card_height - 1 and j == 0)
			{
				gor_card[i][j] = One_LeftDown;
			}
			else if ((i == 0 or i == card_height - 1) and (j >= 1 and j < card_width - 1))
			{
				gor_card[i][j] = One_Gorizont;
			}
			else if ((i >= 1 and i < card_height - 1) and (j == 0 or j == card_width - 1))
			{
				gor_card[i][j] = One_Vertikal;
			}
			else
			{
				if (i % 2 == 0 and j % 2 != 0)
				{
					gor_card[i][j] = 178;
				}
				if (i % 2 != 0 and j % 2 == 0)
				{
					gor_card[i][j] = 178;
				}
				if (i % 2 == 0 and j % 2 == 0)
				{
					gor_card[i][j] = 177;
				}
				if (i % 2 != 0 and j % 2 != 0)
				{
					gor_card[i][j] = 177;
				}
			}
		}
	}

	SetColor(color_1, color_2);
	SetCursor(X, Y);
	for (size_t i = 0; i < card_height; i++)
	{
		SetCursor(X, Y + i);
		for (size_t j = 0; j < card_width; j++)
		{
			cout << gor_card[i][j];
		}
		cout << endl;
	}

	SetColor(White, Black);
	Delete_D_Arr(gor_card, card_height);
}

//Функция отрисовки букв
void Latters_Func(char latter, int X, int Y, Color color_1, Color color_2)
{
	int card_height = 9;
	int card_width = 9;
	char** card_latter = new char* [card_height];
	for (size_t i = 0; i < card_height; i++)
	{
		card_latter[i] = new char[card_width];
	}
	for (size_t i = 0; i < card_height; i++)
	{
		for (size_t j = 0; j < card_width; j++)
		{
			card_latter[i][j] = ' ';
		}
	}
	// Буква Д
	if (latter == 'Д')
	{
		card_latter[1][4] = '$';
		card_latter[2][3] = '$';
		card_latter[2][5] = '$';
		card_latter[3][2] = '$';
		card_latter[3][6] = '$';
		card_latter[4][2] = '$';
		card_latter[4][6] = '$';
		card_latter[5][2] = '$';
		card_latter[5][6] = '$';
		for (size_t i = 0; i < 7; i++)
		{
			card_latter[6][1 + i] = '$';
		}
		card_latter[7][1] = '$';
		card_latter[7][7] = '$';
	}
	// Буква У
	if (latter == 'У')
	{
		card_latter[1][1] = '$';
		card_latter[1][7] = '$';
		card_latter[2][2] = '$';
		card_latter[2][7] = '$';
		card_latter[3][3] = '$';
		card_latter[3][7] = '$';
		card_latter[4][4] = '$';
		card_latter[4][6] = '$';
		card_latter[5][5] = '$';
		card_latter[6][4] = '$';
		card_latter[7][2] = '$';
		card_latter[7][3] = '$';
	}
	// Буква Р
	if (latter == 'Р')
	{
		for (size_t i = 0; i < 7; i++)
		{
			card_latter[1 + i][1] = '$';
		}
		for (size_t i = 0; i < 5; i++)
		{
			card_latter[1][2 + i] = '$';
			card_latter[4][2 + i] = '$';
		}
		card_latter[2][7] = '$';
		card_latter[3][7] = '$';
	}
	// Буква А
	if (latter == 'А')
	{
		for (size_t i = 0; i < 4; i++)
		{
			card_latter[4 + i][1] = '$';
			card_latter[4 + i][7] = '$';
		}
		for (size_t i = 0; i < 5; i++)
		{
			card_latter[5][2 + i] = '$';
		}
		card_latter[1][4] = '$';
		card_latter[2][3] = '$';
		card_latter[2][5] = '$';
		card_latter[3][2] = '$';
		card_latter[3][6] = '$';
	}
	// Буква К
	if (latter == 'К')
	{
		for (size_t i = 0; i < 7; i++)
		{
			card_latter[1 + i][1] = '$';
		}
		for (size_t i = 0; i < 2; i++)
		{
			card_latter[4][2 + i] = '$';
			card_latter[1][6 + i] = '$';
			card_latter[7][6 + i] = '$';

		}
		card_latter[3][4] = '$';
		card_latter[2][5] = '$';
		card_latter[5][4] = '$';
		card_latter[6][5] = '$';
	}
	// Буква И
	if (latter == 'И')
	{

		for (size_t i = 0; i < 7; i++)
		{
			card_latter[1 + i][1] = '$';
			card_latter[1 + i][7] = '$';
		}
		card_latter[6][2] = '$';
		card_latter[5][3] = '$';
		card_latter[4][4] = '$';
		card_latter[3][5] = '$';
		card_latter[2][6] = '$';
	}
	// Буква Г
	if (latter == 'Г')
	{

		for (size_t i = 0; i < 7; i++)
		{
			card_latter[1 + i][1] = '$';
			card_latter[1][1 + i] = '$';
		}
	}
	//Отрисовка буквы
	SetCursor(X, Y);
	for (size_t i = 0; i < card_height; i++)
	{
		SetCursor(X, Y + i);
		for (size_t j = 0; j < card_height; j++)
		{
			if (card_latter[i][j] != ' ')
			{
				SetColor(color_1, color_1);
			}
			else
			{
				SetColor(color_1, color_2);
			}
			cout << card_latter[i][j];
		}
		cout << endl;
	}
	SetColor(White, Black);
	Delete_D_Arr(card_latter, card_height);
}

//Функция раскладки карт на игровом столе(переменная mast  локакльная = 4)
void Print_36_cards_on_table(Koloda* Cards, char** card_ico, int width, int height, int cards_number, int card_nominal)
{
	int mast = 4;
	int x = 14, y = 7;
	bool closed_card = true;

	for (size_t i = 0; i < cards_number; i++)
	{
		PrintCard(Cards[i], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, closed_card);
		Sleep(sleep75);
		x += 13;
		if (i == (cards_number / 3) - 1 or i == (cards_number / 3) * 2 - 1)
		{
			y += 18;
			x = 14;
		}
	}

	Sleep(sleep1000);
	closed_card = false;

	x = 157;
	y = 43;
	for (int i = cards_number - 1; i >= 0; i--)
	{
		PrintCard(Cards[i], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, closed_card);
		Sleep(sleep75);
		x -= 13;
		if (i == (cards_number / 3) or i == (cards_number / 3) * 2)
		{
			y -= 18;
			x = 157;
		}
	}

	SetCursor(73, 55);
	SetColor(White, Red);
	Sleep(sleep1000 * 2);
	system("chcp 1251>NUL");
	cout << "НАЖМИТЕ ЛЮБУЮ КНОПКУ ДЛЯ ПРОДОЛЖЕНИЯ...";
	//cout << "PRESS ANY KEY TO CONTINUE...";
	system("chcp 866>NUL");
	_getch();
	SetCursor(73, 55);
	SetColor(Green, Green);
	cout << "                                       ";
	x = 14;
	y = 7;
	for (size_t i = 0; i < cards_number; i++)
	{
		PrintCard(Cards[i], card_ico, x, y, width, height, (Color)Green, (Color)Green, true);
		Sleep(sleep75);
		x += 13;
		if (i == (cards_number / 3) - 1 or i == (cards_number / 3) * 2 - 1)
		{
			y += 18;
			x = 14;
		}
	}
}

// Функция печати карт с заданным шагом по горизонтали
void Print_players_Cards(Koloda* Current_player_cards, char** card_ico, int number_cards, int X, int Y, int gorizontal_step, bool hidden)
{
	SetCursor(X, Y);
	int step_x = 0;
	for (size_t i = 0; i < number_cards; i++)
	{
		PrintCard(Current_player_cards[i], card_ico, X + step_x, Y, 9, 9, (Color)LightMagenta, (Color)LightCyan, hidden);
		step_x += gorizontal_step;
	}
}

// Функция раздачи карт игрокам
void Print_players_Cards(Koloda* Player_cards, Koloda* Bot_cards, char** card_ico, int number_cards, int X_bot, int Y_bot, int X_player, int Y_player, int gorizontal_step)
{
	int step_xp = 0;
	int step_xb = 0;
	int count_p = 0;
	int count_b = 1;
	for (size_t i = 0; i < number_cards * 2; i++)
	{
		Sleep(sleep300);
		if (i % 2 == 0)
		{
			SetCursor(X_player, Y_player);
			PrintCard(Player_cards[i - count_p], card_ico, X_player + step_xp, Y_player, 9, 9, (Color)LightMagenta, (Color)LightCyan, false);
			step_xp += gorizontal_step;
			count_p++;
		}
		else
		{
			SetCursor(X_bot, Y_bot);
			PrintCard(Bot_cards[i - count_b], card_ico, X_bot + step_xb, Y_bot, 9, 9, (Color)LightMagenta, (Color)LightCyan, true);
			step_xb += gorizontal_step;
			count_b++;
		}
	}
}

//Отрисовка "Игра Дурак"
void Print_Game_durak(Color color_1, Color color_2, bool timeout = true)
{
	int x = 14;
	int x_step = 13;
	int y = 16;
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('И', x, y, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('Г', x + x_step, y, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('Р', x + x_step * 2, y, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('А', x + x_step * 3, y, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('Д', x + x_step * 7, y + 18, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('У', x + x_step * 8, y + 18, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('Р', x + x_step * 9, y + 18, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('А', x + x_step * 10, y + 18, (Color)color_1, (Color)color_2);
	if (timeout)
	{
		Sleep(sleep175);
	}
	Latters_Func('К', x + x_step * 11, y + 18, (Color)color_1, (Color)color_2);
}

//
//БЛОК МЕНЮ
//

bool boolchioce = false;

//Отрисовка главного меню
void MainMenuPrint(string* menuarr, int sizemenu, int menupoint)
{
	int menurow = 11;
	int menucol = 30;
	int Xmenu = 75, Ymenu = 20;
	char** menufone = new char* [menurow];
	for (size_t i = 0; i < menurow; i++)
	{
		menufone[i] = new char[menucol];
	}
	PrintCurrentFone(menufone, menurow, menucol, (Color)White, (Color)Blue, Xmenu, Ymenu);

	int X = 85, Y = 24;
	system("chcp 1251>NUL");
	for (size_t i = 0; i < sizemenu; i++)
	{
		if (i == 1)
		{
			SetCursor(X + 2 * i, Y + i);
		}
		else if (i == 2)
		{
			SetCursor(X + 2, Y + i);
		}
		else
		{
			SetCursor(X, Y + i);
		}

		if (i == menupoint)
		{
			SetColor(Red, White);
		}
		else
		{
			SetColor(White, Blue);
		}

		cout << menuarr[i];
	}
	system("chcp 866>NUL");
	SetColor(White, Black);
	Delete_D_Arr(menufone, menurow);
}
// Выбор пункта меню
int SelectMenuItem(string* menuarr, int menusize)
{
	int menupoint = 0;
	int quit = 2;
	int key;
	//MainMenuPrint(menuarr, menusize, menupoint);
	do
	{
		MainMenuPrint(menuarr, menusize, menupoint);
		key = _getch();
		if (key == Down)
		{
			if (menupoint < menusize)
			{
				menupoint++;
			}
			if (menupoint == menusize)
			{
				menupoint = 0;
			}
		}
		if (key == Up)
		{
			if (menupoint >= 0)
			{
				menupoint--;
			}
			if (menupoint == -1)
			{
				menupoint = menusize - 1;
			}
		}
		if (key == Enter)
		{
			return menupoint;
		}
	} while (key != esc);
	return quit;
}

// Вызов окна создания нового пользователя
void NewPlayerWindow(char** arrfone, int zrow, int zcol, Color zcolor, Color color_1, Color color_2, int nrow, int ncol, int zX = 0, int zY = 0, int nX = 0, int nY = 0)
{
	PrintCurrentFone(arrfone, zrow, zcol, (Color)zcolor, zX, zY);
	PrintCurrentFone(arrfone, nrow, ncol, (Color)color_1, (Color)color_2, nX, nY);
}

// Меню yes/no
//функция подсветки сообщения yes/no и возвращающее значения 1/0
bool PrintChoice(string* chiocearr, int size, int distance = 5, int X = 0, int Y = 0)
{
	int fmenupoint = 0;
	int fkey;
	SetCursor(X, Y);

	do
	{
		system("chcp 1251>NUL");
		for (size_t i = 0; i < size; i++)
		{
			SetCursor(X + i * distance, Y);
			if (i == fmenupoint)
			{
				SetColor(Red, White);
			}
			else
			{
				SetColor(White, Blue);
			}
			cout << chiocearr[i];
			SetColor(White, Black);
		}
		system("chcp 866>NUL");

		fkey = _getch();
		if (fkey == Right)
		{
			if (fmenupoint < size)
			{
				fmenupoint++;
			}
			if (fmenupoint == size)
			{
				fmenupoint = 0;
			}
		}
		if (fkey == Left)
		{
			if (fmenupoint >= 0)
			{
				fmenupoint--;
			}
			if (fmenupoint == -1)
			{
				fmenupoint = size - 1;
			}
		}
		if (fkey == Enter)
		{
			return abs(fmenupoint - 1);
		}
	} while (fkey != esc);
	return 0;
}

// Меню OK содним вариантом действия
void Info_OK(Color color_1, Color color_2, int X = 0, int Y = 0)
{
	int fmenupoint = 0;
	int fkey;
	SetCursor(X, Y);
	SetColor(color_1, color_2);
	cout << "OK";
	SetColor(White, Black);
	do
	{
		fkey = _getch();
		if (fkey == Enter)
		{
			break;
		}
	} while (fkey != esc);
}

// Меню редактирования пользователя
//функция подсветки сообщения create player/delete player/exit  и возвращающее значения 1 2 3
int PrintChoice(string* chiocearr, int size, Color color_1, Color color_2, int distance = 5, int X = 0, int Y = 0)
{
	int fmenupoint = 0;
	int fkey;
	SetCursor(X, Y);

	do
	{
		system("chcp 1251>NUL");
		for (size_t i = 0; i < size; i++)
		{
			SetCursor(X + i * distance, Y);
			if (i == fmenupoint)
			{
				SetColor(Red, White);
			}
			else
			{
				SetColor(White, Blue);
			}
			cout << chiocearr[i];
			SetColor(White, Black);
		}
		system("chcp 866>NUL");

		fkey = _getch();
		if (fkey == Right)
		{
			if (fmenupoint < size)
			{
				fmenupoint++;
			}
			if (fmenupoint == size)
			{
				fmenupoint = 0;
			}
		}
		if (fkey == Left)
		{
			if (fmenupoint >= 0)
			{
				fmenupoint--;
			}
			if (fmenupoint == -1)
			{
				fmenupoint = size - 1;
			}
		}
		if (fkey == Enter)
		{
			return fmenupoint;
		}
	} while (fkey != esc);
	return 2;
}


// Меню выбора пользователя
//функция подсвета пользователя и возвращающая номер слота
int UserChoice(Gamers* GamersTable, int size, Color color_1, Color color_2, int Y = 21)//Y=26
{
	int fmenupoint = 0;
	int fkey;
	ReadFileToGamersTable(GamersTable, size);
	do
	{
		for (size_t i = 0; i < FuncReadF_row_number(temp); i++)
		{
			if (i == fmenupoint)
			{
				SetColor(Red, White);
			}
			else
			{
				SetColor(color_1, color_2);
			}
			SetCursor(70, Y + i);
			cout << GamersTable[i].user_id + 1;
			SetCursor(79, Y + i);
			cout << GamersTable[i].name;
			SetCursor(96, Y + i);
			cout << GamersTable[i].games;
			SetCursor(109, Y + i);
			cout << GamersTable[i].wins << endl;
			SetColor(White, Black);
		}

		fkey = _getch();
		if (fkey == Down)
		{
			if (fmenupoint < size)
			{
				fmenupoint++;
			}
			if (fmenupoint == size)
			{
				fmenupoint = 0;
			}
		}
		if (fkey == Up)
		{
			if (fmenupoint >= 0)
			{
				fmenupoint--;
			}
			if (fmenupoint == -1)
			{
				fmenupoint = size - 1;
			}
		}
		if (fkey == Enter)
		{
			return fmenupoint;
		}
	} while (fkey != esc);
	return -1;
}

//Функция проверки пароля
bool PasswordCheck(Gamers* GamersTable, string pass, int slot_number)
{
	if (GamersTable[slot_number].password == pass)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
// Блок логики
// 

//Обнуление строки карт
void NullCard(Koloda* Card, int i)
{
	Card[i].number = 0;
	Card[i].suit = (char)90;
	Card[i].name = 'Z';
	Card[i].trump = false;
}

//Обнуление пула карт
void NullPoolCards(Koloda* Pool, int number_cards)
{
	for (size_t i = 0; i < number_cards; i++)
	{
		NullCard(Pool, i);
	}
}

// Обзывание карт
void CardName(Koloda& Card, int card_number, char suit)
{
	//if (Card.number == 0)
	//{
	//	Card.name = "SIX";
	//}
	//if (Card.number == 1)
	//{
	//	Card.name = "SEVEN";
	//}
	//if (Card.number == 2)
	//{
	//	Card.name = "EIGHT";
	//}
	//if (Card.number == 3)
	//{
	//	Card.name = "NINE";
	//}
	//if (Card.number == 4)
	//{
	//	Card.name = "TEN";
	//}
	//if (Card.number == 5)
	//{
	//	Card.name = "JACK";
	//}
	//if (Card.number == 6)
	//{
	//	Card.name = "QUEEN";
	//}
	//if (Card.number == 7)
	//{
	//	Card.name = "KING";
	//}
	//if (Card.number == 8)
	//{
	//	Card.name = "ACE";
	//}
	//if (suit == (char)3)
	//{
	//	Card.name += " HEARTS";
	//}
	//if (suit == (char)4)
	//{
	//	Card.name += " DIAMONDS";
	//}
	//if (suit == (char)5)
	//{
	//	Card.name += " CLUBS";
	//}
	//if (suit == (char)6)
	//{
	//	Card.name += " SPADES";
	//}

	if (Card.number == 0)
	{
		Card.name = RUCIN("ШЕСТЬ");
	}
	if (Card.number == 1)
	{
		Card.name = RUCIN("СЕМЬ");
	}
	if (Card.number == 2)
	{
		Card.name = RUCIN("ВОСЕМЬ");
	}
	if (Card.number == 3)
	{
		Card.name = RUCIN("ДЕВЯТЬ");
	}
	if (Card.number == 4)
	{
		Card.name = RUCIN("ДЕСЯТЬ");
	}
	if (Card.number == 5)
	{
		Card.name = RUCIN("ВАЛЕТ");
	}
	if (Card.number == 6)
	{
		Card.name = RUCIN("ДАМА");
	}
	if (Card.number == 7)
	{
		Card.name = RUCIN("КОРОЛЬ");
	}
	if (Card.number == 8)
	{
		Card.name = RUCIN("ТУЗ");
	}
	if (suit == (char)3)
	{
		Card.name += RUCIN(" ЧЕРВЕЙ");
	}
	if (suit == (char)4)
	{
		Card.name += RUCIN(" БУБЕЙ");
	}
	if (suit == (char)5)
	{
		Card.name += RUCIN(" КРЕСТЕЙ");
	}
	if (suit == (char)6)
	{
		Card.name += RUCIN(" ПИКЕЙ");
	}
}
//Генерация колоды карт
void KolodaGenerator(Koloda* Cards, int suit, int card_number, int card_nominal)
{

	for (size_t i = 0; i < suit; i++)
	{
		for (size_t i = 0; i < card_number; i++)
		{
			//Черви
			if (i >= 0 and i < 9)
			{

				Cards[i].number = i;
				Cards[i].suit = (char)3;
				Cards[i].trump = false;
				CardName(Cards[i], i, 3);

			}
			//Буби
			if (i >= 9 and i < 18)
			{

				Cards[i].number = i - card_nominal;
				Cards[i].suit = (char)4;
				Cards[i].trump = false;
				CardName(Cards[i], i, 4);

			}
			//Крести
			if (i >= 18 and i < 27)
			{

				Cards[i].number = i - card_nominal * 2;
				Cards[i].suit = (char)5;
				Cards[i].trump = false;
				CardName(Cards[i], i, 5);
			}
			//Пики
			if (i >= 27 and i < 36)
			{
				Cards[i].number = i - card_nominal * 3;
				Cards[i].suit = (char)6;
				Cards[i].trump = false;
				CardName(Cards[i], i, 6);
			}
		}
	}
}

//Перетасовывание карт в колоде
void KolodaMix(Koloda* cards, int card_number, int mix_rounds)
{
	for (size_t i = 0; i < mix_rounds; i++)
	{
		swap(cards[rand() % (card_number / 2)], cards[card_number / 2 + rand() % (card_number / 2)]);
	}
}

//Объявление козырей
void Trump_13(Koloda* Cards, int trump_card = 12, int card_number = 36)
{

	for (size_t i = 0; i < card_number; i++)
	{
		if (Cards[i].suit == Cards[trump_card].suit)
		{
			Cards[i].trump = true;
			//Cards[i].name.insert(0, "TRUMP ");

			Cards[i].name.insert(0, RUCIN("КОЗЫРЬ "));
		}
	}

}

// Функция раздачи карт боту и игроку
void RazdachaCards(Koloda* Cards, Koloda* Player_cards, Koloda* Bot_cards, int card_number, int player_pool_card_number)
{
	int jp = 0;
	int jb = 0;
	for (size_t i = 0; i < player_pool_card_number * 2; i++)
	{
		if (i % 2 == 0)
		{
			Player_cards[jp] = Cards[i];
			NullCard(Cards, i);
			jp++;
		}
		if (i % 2 != 0)
		{
			Bot_cards[jb] = Cards[i];
			NullCard(Cards, i);
			jb++;
		}
	}
}

// Перемещение объявленного козыря в конец колоды
void SwapTrump(Koloda* Cards, int trump_number, int last_card_number)
{
	swap(Cards[trump_number], Cards[last_card_number]);
}

//Функция определения первого хода - возвращает 10+номинал карты если первым ходит игрок, 20+номинал карты - если ходит бот
int First_move(Koloda* Player_cards, Koloda* Bot_cards, int cards_per_player)
{
	int player_trump_pow = 9;
	int bot_trump_pow = 9;
	//int temp_pow;

	for (size_t i = 0; i < cards_per_player; i++)
	{
		if (Player_cards[i].trump and Player_cards[i].number < player_trump_pow)
		{
			player_trump_pow = Player_cards[i].number;
		}
		if (Bot_cards[i].trump and Bot_cards[i].number < bot_trump_pow)
		{
			bot_trump_pow = Bot_cards[i].number;
		}
	}

	//Первым ходит игрок
	if (player_trump_pow < bot_trump_pow)
	{
		return 10 + player_trump_pow;
	}
	//Первым ходит бот
	else
	{
		return 20 + bot_trump_pow;
	}
}

//Функция счетчика остатка карт в колоде
int Cards_in_deck(Koloda* Cards, int number_cards = 36)
{

	int count = 0;
	for (size_t i = 0; i < number_cards; i++)
	{
		if (Cards[i].suit != 'Z')
		{
			count++;
		}
	}
	return count;
}

// Функция сортировки пустых слотов руки
void Sort_Pool_Cards(Koloda* Cards, int number_cards)
{
	for (size_t i = 0; i < number_cards - 1; i++)
	{
		if (Cards[i].suit == 'Z' and Cards[i + 1].suit != 'Z')
		{
			swap(Cards[i], Cards[i + 1]);
		}
	}
}

//Функция выбора карты, возвращает номер карты в руке игрока, визуально стирает выбранную карту, если нажато esc возвращает -1
int Chioce_Player_Card(Koloda* Player_cards, char** card_ico, char **fone, int X, int Y, int step, int def_step = 11)
{
	int i = 0;
	int number_card_in_deck = Cards_in_deck(Player_cards);
	int fkey;
	int f_step = 0;
	int sdvig = 1;
	int temp_step = step;
	do
	{

		// Отрисовка карты - отрисовывает первую карту сверху, затирает эту же карту снизу
		Y = 49;
		PrintCard(Player_cards[i], card_ico, X + f_step, Y, 9, 9, (Color)Green, (Color)Green, true);
		Y = 39;
		PrintCard(Player_cards[i], card_ico, X + f_step, Y, 9, 9, (Color)Green, (Color)Green, false);
		fkey = _getch();
		if (fkey == Right)
		{
			i++;
			f_step += step;
			//Перенос в начало
			if (i == number_card_in_deck)
			{
				PrintCard(Player_cards[i - 1], card_ico, X + f_step - step, Y, 9, 9, (Color)Green, (Color)Green, true);
				PrintCard(Player_cards[i - 1], card_ico, X + f_step - step, Y + 10, 9, 9, (Color)Green, (Color)Green, false);
				i = 0;
				f_step = 0;
				sdvig = 0;
				temp_step = 0;
			}
			//Отрисовывается карта, которая была на позиции i-1 снизу
			PrintCard(Player_cards[i], card_ico, X + f_step - temp_step, Y, 9, 9, (Color)Green, (Color)Green, true);
			//Затирается карта, которая была на позиции i-1 сверх
			PrintCard(Player_cards[i - sdvig], card_ico, X + f_step - temp_step, Y + 10, 9, 9, (Color)Green, (Color)Green, false);
			sdvig = 1;
			temp_step = step;
		}
		if (fkey == Left)
		{
			i--;
			f_step -= step;
			//Перенос в конец
			if (i == -1)
			{
				PrintCard(Player_cards[i + 1], card_ico, X + f_step + step, Y, 9, 9, (Color)Green, (Color)Green, true);
				PrintCard(Player_cards[i + 1], card_ico, X + f_step + step, Y + 10, 9, 9, (Color)Green, (Color)Green, false);
				i = number_card_in_deck - 1;
				f_step = number_card_in_deck * step - step;
				sdvig = 0;
				temp_step = 0;
			}
			PrintCard(Player_cards[i], card_ico, X + f_step + temp_step, Y, 9, 9, (Color)Green, (Color)Green, true);
			PrintCard(Player_cards[i + sdvig], card_ico, X + f_step + temp_step, Y + 10, 9, 9, (Color)Green, (Color)Green, false);
			sdvig = 1;
			temp_step = step;
		}
		if (fkey == esc)
		{
			return -1;
		}
	} while (fkey != Enter);
	PrintCard(Player_cards[i], card_ico, X + f_step, Y, 9, 9, (Color)Green, (Color)Green, true);
	return i;
}

//Функция заполнения пула карт по передаваемой карте
void Fill_Pool_Cards(Koloda* Input_Pool_Cards, Koloda* Output_Pool_Cards, int number_cards, int card_i)
{
	for (size_t i = 0; i < number_cards; i++)
	{
		if (Input_Pool_Cards[i].suit == 'Z')
		{
			Input_Pool_Cards[i] = Output_Pool_Cards[card_i];
			break;
		}
	}
}

//Функция ответа игрока на карту бота Если карта которой пытается отбиться игрок больше по номиналу  и/или козырь возвращается 1, если отбиться картой с пришедшим индексом нельзя возвращается 0
int Player_Answer(Koloda* Player_cards, Koloda* Battlefield, int player_card_index)
{
	int i = 0;
	int number_cards_on_deck = Cards_in_deck(Battlefield);
	int number_cards_in_player_hand = Cards_in_deck(Player_cards);
	int temp_k_card = -1;
	int temp_nominal_card = 9;
	for (size_t i = 0; i < number_cards_on_deck; i++)
	{
		//Если карта на поле подкинута ботом для отбивания
		if (i % 2 == 0 and i + 1 == number_cards_on_deck)
		{
			//Карта, подброшенная ботом - козырь
			if (Battlefield[i].trump)
			{
				if (Player_cards[player_card_index].number > Battlefield[i].number and Player_cards[player_card_index].trump)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			// Карта, подброшенная ботом не козырь
			else
			{
				if (Player_cards[player_card_index].trump)
				{
					return 1;
				}
				else if (Player_cards[player_card_index].suit == Battlefield[i].suit and Player_cards[player_card_index].number > Battlefield[i].number)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}
}

//Функция ответа бота на карту игрока
int Bot_Answer_Level_0(Koloda* Bot_cards, Koloda* Battlefield)
{
	int i = 0;
	int number_cards_on_deck = Cards_in_deck(Battlefield);
	int number_cards_in_bot_hand = Cards_in_deck(Bot_cards);
	int temp_k_card = -1;
	int temp_nominal_card = 9;
	//int card_nominal;

	for (size_t i = 0; i < number_cards_on_deck; i++)
	{
		//Если карта на поле подкинута игроком для отбивания
		if (i % 2 == 0 and i + 1 == number_cards_on_deck)
		{
			// Если j-я карта руки бота одной и той-же масти с подкинутой картой и подкинутая карта НЕ КОЗЫРЬ!
			for (size_t j = 0; j < number_cards_in_bot_hand; j++)
			{
				if (Bot_cards[j].suit == Battlefield[i].suit and Battlefield[i].trump == false)
				{
					if (Bot_cards[j].number < temp_nominal_card and Bot_cards[j].number > Battlefield[i].number)
					{
						temp_nominal_card = Bot_cards[j].number;
						temp_k_card = j;
					}
				}

			}
			if (temp_k_card != -1)
			{
				return temp_k_card;
			}

			// Если подкинутая карта не козырь, но в руке бота нет масти для и/или соответствующего номинала чтобы ее крыть - ПРОВЕРКА НА ВОЗМОЖНОСТЬ ПОКРЫТЬ КАРТУ КОЗЫРЕМ
			temp_nominal_card = 9;
			for (size_t j = 0; j < number_cards_in_bot_hand; j++)
			{
				//Если в руке бота нет масти некозырной карты которую надо отбить - бот пытается отбиться козырем
				if (Battlefield[i].trump == false and Bot_cards[j].trump == true)
				{
					if (Bot_cards[j].number < temp_nominal_card)
					{
						temp_nominal_card = Bot_cards[j].number;
						temp_k_card = j;
					}
				}
			}
			if (temp_k_card != -1)
			{
				return temp_k_card;
			}
			// Если подкинутая карта козырь. ПРОВЕРКА НА ВОЗМОЖНОСТЬ ОТБИТЬСЯ КОЗЫРЕМ
			temp_nominal_card = 9;
			for (size_t j = 0; j < number_cards_in_bot_hand; j++)
			{
				if (Battlefield[i].trump == true and Bot_cards[j].trump == true)
				{
					if (Bot_cards[j].number < temp_nominal_card and Bot_cards[j].number > Battlefield[i].number)
					{
						temp_nominal_card = Bot_cards[j].number;
						temp_k_card = j;
					}
				}
			}
			if (temp_k_card != -1)
			{
				return temp_k_card;
			}
			// Если бот не может побить подкинутую карту
			return -1;
		}
	}
}

//Функция ход бота turn_on - для проверки можно ли подбрасывать
int Bot_Turn_Level_0(Koloda* Bot_cards, Koloda* Battlefield, Koloda* Cards, bool turn_on)
{
	int min_nominal = 9;
	int index = -1;
	bool only_trumps = true;

	for (size_t i = 0; i < Cards_in_deck(Bot_cards); i++)
	{
		if (Bot_cards[i].trump == false)
		{
			only_trumps = false;
		}
	}

	//Не ходит с козырей
	if (Cards_in_deck(Cards) > 0)
	{
		for (size_t i = 0; i < Cards_in_deck(Bot_cards); i++)
		{
			if (Cards_in_deck(Battlefield) == 0)
			{
				if (Bot_cards[i].trump == false and Bot_cards[i].number < min_nominal)
				{
					min_nominal = Bot_cards[i].number;
					index = i;
				}
				else if (only_trumps)
				{
					if (Bot_cards[i].number < min_nominal)
					{
						min_nominal = Bot_cards[i].number;
						index = i;
					}
				}
			}
			//проверка подбрасывания
			else
			{
				//Если карта не пустая и не козырь
				if (Bot_cards[i].suit != 'Z' and Bot_cards[i].trump == false)
				{
					//Можно ли подбросить карту
					for (size_t j = 0; j < Cards_in_deck(Battlefield); j++)
					{
						if (Bot_cards[i].number == Battlefield[j].number)
						{
							return i;
						}
					}
				}
			}

		}
	}
	//Карты в колоде закончились Бот может ходить с козырей
	else
	{
		for (size_t i = 0; i < Cards_in_deck(Bot_cards); i++)
		{
			if (Cards_in_deck(Battlefield) == 0)
			{
				if (Bot_cards[i].number < min_nominal)
				{
					min_nominal = Bot_cards[i].number;
					index = i;
				}
			}
			//проверка подбрасывания с возможностью подбрасывания козырей
			else
			{
				//Если карта не пустая
				if (Bot_cards[i].suit != 'Z')
				{
					//Можно ли подбросить карту
					for (size_t j = 0; j < Cards_in_deck(Battlefield); j++)
					{
						if (Bot_cards[i].number == Battlefield[j].number)
						{
							return i;
						}
					}
				}
			}
		}
	}
	return index;
}

//Функция определения начальной координаты карты и шага отрисовки. Возвращает код Пример - 8011 где 80 начальная координата, 11 шаг 
int X_Cards_Coordinat_and_Step(int number_cards, int X_step = 11, int width = 178, int width_card = 9)
{
	int start_X = (width - ((number_cards - 1) * X_step + width_card)) / 2;
	if (start_X < 10)
	{
		start_X = 10;
	}
	int end_X;
	int lenthg;
	do
	{
		end_X = start_X + X_step * (number_cards - 1) + width_card;
		lenthg = end_X - start_X;
		if (lenthg > 160)
		{
			X_step--;
		}
		else
		{
			break;
		}
	} while (true);

	return (start_X * 100 + X_step);
}

//Функция проверки возможности хода (Если на столе нет карт - возвращает 1, если на столе есть карты, проверяет есть ли соответствующий номинал в руке игрока. Если есть - возвращает 1, если нет возвращает 0)
int Test_Player_turn_on(Koloda* Battlefield, Koloda* Current_Player_cards, int number_cards = 36)
{
	if (Cards_in_deck(Battlefield) == 0)
	{
		return 1;
	}
	else
	{
		for (size_t i = 0; i < Cards_in_deck(Current_Player_cards); i++)
		{
			if (Current_Player_cards[i].suit != 'Z')
			{
				for (size_t j = 0; j < Cards_in_deck(Battlefield); j++)
				{
					if (Current_Player_cards[i].number == Battlefield[j].number)
					{
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

//Функция БИТО Передает карты со стола в отбой, возвращает 1 - ход игрока или 0 - ход бота НЕ ОТРИСОВЫВАЕТ НИЧЕГО!
int bito(Koloda* Battlefield, Koloda* Played_cards, bool player_turn, int number_cards = 36)
{
	for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
	{
		Fill_Pool_Cards(Battlefield, Played_cards, number_cards, i);
	}
	NullPoolCards(Battlefield, number_cards);
	return !player_turn;
}

//Функция отрисовки отбоя
void Print_Played_Card(Koloda* Played_Cards, char** card_ico, int n = 2)
{
	int numbercards = 1;
	int step = 1;
	int X_play;
	int Y_play;
	for (size_t i = 0; i < n; i++)
	{
		X_play = 162 + rand() % 6;
		Y_play = 23 + rand() % 6;
		Print_players_Cards(Played_Cards, card_ico, numbercards, X_play, Y_play, step, true);
	}
}

// Функция добора карт. Если в колоде добора есть карты, передает карты из колоды добора в руки игроков если карт в руке < 6 Первым осуществляет добор тот игрок который ходил. Реализовать - Если у обоих игроков количество карт равно 0, отбивавшийся игрок берет последнюю карту из добора 
void Dobor(Koloda* Cards, Koloda* Player_cards, Koloda* Bot_cards, bool pt, int cards_per_player = 6, int number_cards = 36)
{
	int card_to_take;
	int temp_cart = 0;
	int i_card = 0;
	int p_card = 0;
	int b_card = 0;
	int count = 0;

	int cards_need_player = cards_per_player - Cards_in_deck(Player_cards);
	int cards_need_bot = cards_per_player - Cards_in_deck(Bot_cards);
	bool take = true;
	int dobor_nechet = 0;

	if (Cards_in_deck(Cards) > 0)
	{
		// Пока все не набрали карты
		do
		{
			// Определение iй карты с которой надо начинать прилепливать карты из колоды
			for (size_t i = 0; i < number_cards; i++)
			{
				if (Player_cards[i].suit == 'Z')
				{
					p_card = i;
					break;
				}
			}
			for (size_t i = 0; i < number_cards; i++)
			{
				if (Bot_cards[i].suit == 'Z')
				{
					b_card = i;
					break;
				}
			}
			// Добор карт игроком
			if (pt)
			{
				//Нахождение индекса первой непустой карты
				for (size_t i = 0; i < number_cards; i++)
				{
					if (Cards[i].suit != 'Z')
					{
						i_card = i;
						break;
					}
				}
				// Проверка добора
				if (Cards_in_deck(Player_cards) < cards_per_player)
				{
					// Если количество карт в колоде больше суммарно необходимого игорокам - игроки берут карт столько сколько нужно
					if (Cards_in_deck(Cards) >= (cards_need_player + cards_need_bot))
					{
						card_to_take = cards_per_player - Cards_in_deck(Player_cards);
						take = false;
					}
					// В колоде карт меньше, чем нужно игрокам И игроку и боту нужно добрать карты - количество карт в колоде делится на 2 +1 вслучае если нечтное количество карт осталось в колоде /2 - если четное
					else if (Cards_in_deck(Cards) < (cards_need_player + cards_need_bot))
					{
						// Если количество карт оставшихся в колоде нечетное
						if (take and Cards_in_deck(Cards) % 2 != 0)
						{
							(card_to_take = Cards_in_deck(Cards) / 2) + 1;
							take = false;
						}
						// Если количество карт в колоде четное
						else if (take and Cards_in_deck(Cards) % 2 == 0)
						{
							card_to_take = Cards_in_deck(Cards) / 2;
							take = false;
						}
						else
						{
							card_to_take = Cards_in_deck(Cards);
						}
					}
					//Если в колоде карт больше или равно количеству карт, которые требуется добрать
					if (card_to_take <= Cards_in_deck(Cards))
					{
						for (size_t i = 0; i < card_to_take; i++)
						{
							Player_cards[i + p_card] = Cards[i + i_card];
							NullCard(Cards, i + i_card);
						}
					}
					// Если в колоде карт недостаточно
					else
					{
						temp_cart = Cards_in_deck(Cards);
						for (size_t i = 0; i < temp_cart; i++)
						{
							Player_cards[i + p_card] = Cards[i + i_card];
							NullCard(Cards, i + i_card);
						}
					}
				}
				if (Cards_in_deck(Cards) == 0)
				{
					break;
				}
				count++;
				pt = false;
			}
			// Добор карт ботом
			if (!pt and count < 2)
			{
				//Нахождение индекса первой непустой карты
				for (size_t i = 0; i < number_cards; i++)
				{
					if (Cards[i].suit != 'Z')
					{
						i_card = i;
						break;
					}
				}
				// Проверка добора
				if (Cards_in_deck(Bot_cards) < cards_per_player)
				{
					// Если количество карт в колоде больше суммарно необходимого игорокам - игроки берут карт столько сколько нужно
					if (Cards_in_deck(Cards) >= (cards_need_player + cards_need_bot))
					{
						card_to_take = cards_per_player - Cards_in_deck(Bot_cards);
						take = false;
					}
					// В колоде карт меньше, чем нужно игрокам И игроку и боту нужно добрать карты - количество карт в колоде делится на 2 +1 вслучае если нечтное количество карт осталось в колоде /2 - если четное
					else if (Cards_in_deck(Cards) < (cards_need_player + cards_need_bot))
					{
						// Если количество карт оставшихся в колоде нечетное
						if (take and Cards_in_deck(Cards) % 2 != 0)
						{
							(card_to_take = Cards_in_deck(Cards) / 2) + 1;
							take = false;
						}
						// Если количество карт в колоде четное
						else if (take and Cards_in_deck(Cards) % 2 == 0)
						{
							card_to_take = Cards_in_deck(Cards) / 2;
							take = false;
						}
						else
						{
							card_to_take = Cards_in_deck(Cards);
						}
					}

					//Если в колоде карт больше или равно количеству карт, которые требуется добрать
					if (card_to_take <= Cards_in_deck(Cards))
					{
						for (size_t i = 0; i < card_to_take; i++)
						{
							Bot_cards[i + b_card] = Cards[i + i_card];
							NullCard(Cards, i + i_card);
						}
					}
					// Если в колоде карт недостаточно
					else
					{
						temp_cart = Cards_in_deck(Cards);
						for (size_t i = 0; i < temp_cart; i++)
						{
							Bot_cards[i + b_card] = Cards[i + i_card];
							NullCard(Cards, i + i_card);
						}
					}
				}
				if (Cards_in_deck(Cards) == 0)
				{
					break;
				}
				count++;
				pt = true;
			}
		} while (count != 2);
	}
}

// Фннкция записи номиналов карт с кона(battlefield)
void ReadToTempIntArr(Koloda* Battlefield, int* temp_nominal_arr, int temp_number_cards = 9)
{
	int temp_number;
	bool key = true;
	// Контрольное заминусовывание массиива
	for (size_t i = 0; i < temp_number_cards; i++)
	{
		temp_nominal_arr[i] = -1;
	}

	// Запись уникальных номиналов карт с battlefiled
	for (int i = 0, k = 0; i < Cards_in_deck(Battlefield); i++)
	{
		key = true;
		temp_number = Battlefield[i].number;
		for (int j = 0; j < temp_number_cards; j++)
		{
			if (temp_number == temp_nominal_arr[j])
			{
				key = false;
			}
		}
		if (key)
		{
			temp_nominal_arr[k] = temp_number;
			k++;
		}
	}
}

// Функция перебора руки игрока. Если номинал на кону совпадает с i-й картой игрока - выводится сообщение. Если игрок подтверждает - карта передается боту, если нет - переход к следующей i-й карте
void Podbros_To_Bot(Koloda* Player_cards, Koloda* Bot_cards, Koloda* Battlefield, int* temp_nominal_arr, char** fone, int number_cards = 36, int temp_number_cards = 9, int cards_per_player = 6)
{
	int bot_number_cards = Cards_in_deck(Bot_cards);
	int player_temp_number_cards = 0;
	bool key = true;
	bool player_chioce = false;
	int index = 0;

	const int yes_no = 2;
	system("chcp 1251>NUL");
	string yesno[yes_no]{ "ДА", "НЕТ" };
	system("chcp 866>NUL");

	// Вычисление максимально возможного количества подкидываемых карт если их количество меньше 6ти
	for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
	{
		if (i % 2 != 0)
		{
			bot_number_cards++;
		}
	}
	// Вычисление количества уже подброшенных карт
	for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
	{
		if (i % 2 == 0)
		{
			player_temp_number_cards++;
		}
	}
	// Пока можно подбрасывать - суммарное число карт не больше 6ти или не больше начального количества карт бота

	for (size_t i = 0; i < Cards_in_deck(Player_cards); i++)
	{
		//Подброс карт
		for (int j = 0; j < temp_number_cards; j++)
		{
			if (Player_cards[i].number == temp_nominal_arr[j])
			{
				//Карту подбросить можно
				key = true;
				index = i;
				break;
			}
			else
			{
				// Карту подбросить нельзя
				key = false;
			}
		}
		if (key and player_temp_number_cards != bot_number_cards and player_temp_number_cards <= cards_per_player)
		{
			do
			{
				// информационное окно с функцией выбора
				PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Blue, 60, 14);
				SetColor(White, Blue);
				SetCursor(72, 15);
				system("chcp 1251>NUL");
				cout << "БОТ ВЗЯЛ КАРТЫ. ХОТИТЕ ПОДБРОСИТЬ?";
				system("chcp 866>NUL");
				SetCursor(82, 16);
				cout << Player_cards[index].name;
				//При нажатии yes PrintChoice возвращает 1
				player_chioce = PrintChoice(yesno, 2, 38, 70, 17);
				//Стирает сообщение
				PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
				// Если функция вернула 1 игрок подбросил иначе - проверяю  следующую карту в руке игрока
				if (player_chioce)
				{
					Fill_Pool_Cards(Bot_cards, Player_cards, number_cards, index);
					NullCard(Player_cards, index);
					Sort_Pool_Cards(Player_cards, number_cards);
					i--;
					player_temp_number_cards++;
					break;
				}
				//Если отказался подбрасыать
				else
				{
					key = false;
				}

				// Условия выхода из цикла
				if (player_temp_number_cards == cards_per_player)
				{
					break;
				}
				else if (player_temp_number_cards == bot_number_cards)
				{
					break;
				}
				else if (!key)
				{
					break;
				}
			} while (true);
		}
	}
}

// Функция перебора руки БОТА. Если номинал на кону совпадает с i-й картой бота - бот с определенной вероятностью подкидывает карту игроку. Выводится сообщение - бот подкинул такую-то карту. 
void Podbros_To_Player(Koloda *Cards, Koloda* Player_cards, Koloda* Bot_cards, Koloda* Battlefield, int* temp_nominal_arr, char** fone, int number_cards = 36, int temp_number_cards = 9, int cards_per_player = 6)
{
	Sort_Pool_Cards(Bot_cards, number_cards);
	int player_number_cards = Cards_in_deck(Player_cards);
	int bot_temp_number_cards = 0;
	bool key = true;
	bool bot_chioce = false;
	int index = 0;

	// Вычисление максимально возможного количества подкидываемых карт если их количество меньше 6ти
	for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
	{
		if (i % 2 != 0)
		{
			player_number_cards++;
		}
	}
	// Вычисление количества уже подброшенных карт
	for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
	{
		if (i % 2 == 0)
		{
			bot_temp_number_cards++;
		}
	}
	// Пока можно подбрасывать - суммарное число карт не больше 6ти или не больше начального количества карт бота
	for (size_t i = 0; i < Cards_in_deck(Bot_cards); i++)
	{
		Sort_Pool_Cards(Bot_cards, number_cards);
		//Подброс карт ботом
		for (int j = 0; j < temp_number_cards; j++)
		{
			if (Bot_cards[i].number == temp_nominal_arr[j])
			{
				//Карту подбросить можно
				key = true;
				index = i;
				break;

			}
			else
			{
				// Карту подбросить нельзя
				key = false;
			}
		}

		//cout << Bot_cards[i].name << " nominal " << Bot_cards[i].number << " ";


		if (key and bot_temp_number_cards != player_number_cards and bot_temp_number_cards <= cards_per_player)
		{
			do
			{
				// Если количество карт в доборе > 0
				if (Cards_in_deck(Cards)>0)
				{
					// Вычисляиется вероятность подбрасывания
					if (rand()%100 < chance_to_drop)
					{
						// Если карта козырь - бот не покидывает
						if (Bot_cards[index].trump)
						{
							bot_chioce = false;
						}
						else
						{
							bot_chioce = true;
						}
					}
				}
				// Если карт в доборе 0 - бот подкидывает всегда все что может
				else
				{
					bot_chioce = true;
				}

				if (bot_chioce)
				{
					//Информационное сообщение
					PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
					SetColor(White, Red);
					SetCursor(76, 15);
					system("chcp 1251>NUL");
					cout << "БОТ ПОДБРАСЫВАЕТ ВАМ КАРТЫ:";
					system("chcp 866>NUL");
					SetCursor(82, 16);
					cout << Bot_cards[index].name;
					Info_OK((Color)Red, (Color)White, 88, 17);
					PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
				}
				// Если бот решил подбросить карту - bot_chioce = 1 иначе - проверяю  следующую карту в руке игрока
				if (bot_chioce)
				{
					Fill_Pool_Cards(Player_cards, Bot_cards, number_cards, index);
					NullCard(Bot_cards, index);
					Sort_Pool_Cards(Bot_cards, number_cards);
					i--;
					bot_temp_number_cards++;
					break;
				}
				//Если отказался подбрасыать
				else
				{
					key = false;
				}

				// Условия выхода из цикла
				if (bot_temp_number_cards == cards_per_player)
				{
					break;
				}
				else if (bot_temp_number_cards == player_number_cards)
				{
					break;
				}
				else if (!key)
				{
					break;
				}
			} while (true);
		}
	}
}

//!!!!!!!!!!!!
// Game func
//!!!!!!!!!!!!

void GameFunc(Koloda* Cards, Koloda* Player_cards, Koloda* Bot_cards, Koloda* Played_cards, Koloda* Battlefield, Gamers* GameTable, int number_cards, int card_per_payer, char** card_ico, int x, int y, int width, int height, int activeuserslot, char** fone, int fone_row, int fone_col)
{
	int X_current_card = 57;
	int Y_current_card;
	int temp_i = 0;
	int X_bot_start = 57;
	int Y_bot_start = 2;
	int X_player_start = 57;
	int Y_player_start = 49;
	int X_step = 11;
	//int round = 0;
	int card_nominal = 9;
	// записывается количество карт для проверки срабатывания блока БИТО
	int temp_number_player_card = 0;
	int botanswer;
	bool player_otbilsya = true;
	bool end_con = false;
	int number_player_games = 0;
	int wins = 0;

	// Массив для хранения номиналов - используется при подкидывании карт после того как игрок/бот взял карты
	int* temp_nominal_battlefield_cards = new int[card_nominal];

	const int yes_no = 2;
	system("chcp 1251>NUL");
	string rechange[yes_no]{ "ПЕРЕВЫБРАТЬ", "ЗАВЕРШИТЬ ХОД" };
	string yesno[yes_no]{ "ДА", "НЕТ" };
	system("chcp 866>NUL");

	PrintCurrentFone(fone, fone_row, fone_col, (Color)Green, 1, 1);
	Print_players_Cards(Player_cards, Bot_cards, card_ico, card_per_payer, X_bot_start, Y_bot_start, X_player_start, Y_player_start, X_step);
	Sleep(sleep300);
	PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
	Sleep(sleep1000);
	PrintSuitGorizont(4, 29, (Color)LightMagenta, (Color)LightCyan);
	Sleep(sleep1000 / 5);

	// количество карт в колоде
	SetCursor(5, 35);
	SetColor(Red, White);
	system("chcp 1251>NUL");
	cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
	system("chcp 866>NUL");

	//Предъявление козырной карты
	if (First_move(Player_cards, Bot_cards, card_per_payer) < 20)
	{
		int card_number = First_move(Player_cards, Bot_cards, card_per_payer) - 10;
		for (size_t i = 0; i < card_per_payer; i++)
		{
			if (Player_cards[i].trump and Player_cards[i].number == card_number)
			{
				X_step *= i;
				temp_i = i;
			}
		}
		X_current_card += X_step;
		Y_current_card = 49;
		Sleep(sleep1000 * 2);
		PrintCard(Player_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, true);
		Y_current_card = 39;
		PrintCard(Player_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, false);
		SetCursor(82, 30);
		SetColor(Red, White);
		system("chcp 1251>NUL");
		cout << "ХОД ИГРОКА!";
		system("chcp 866>NUL");
		_getch();
		PrintCard(Player_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, true);
		Y_current_card = 49;
		PrintCard(Player_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, false);
		SetCursor(82, 30);
		SetColor(Green, Green);
		cout << "            ";
		player_otbilsya = true;
		//bot_otbilsya = false;
	}
	else
	{
		int card_number = First_move(Player_cards, Bot_cards, card_per_payer) - 20;
		for (size_t i = 0; i < card_per_payer; i++)
		{
			if (Bot_cards[i].trump and Bot_cards[i].number == card_number)
			{
				X_step *= i;
				temp_i = i;
			}
		}
		X_current_card += X_step;
		Y_current_card = 2;
		Sleep(sleep1000 * 2);
		PrintCard(Bot_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, true);
		Y_current_card = 12;
		PrintCard(Bot_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, false);
		SetCursor(84, 30);
		SetColor(Red, White);
		system("chcp 1251>NUL");
		cout << "ХОД БОТА!";
		system("chcp 866>NUL");
		_getch();
		PrintCard(Bot_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)Green, (Color)Green, true);
		Y_current_card = 2;
		PrintCard(Bot_cards[temp_i], card_ico, X_current_card, Y_current_card, width, height, (Color)LightMagenta, (Color)LightCyan, true);
		SetCursor(84, 30);
		SetColor(Green, Green);
		cout << "            ";
		player_otbilsya = false;
		//bot_otbilsya = true;
	}

	//Краткие правила
	SetCursor(105, 58);
	SetColor(Black, Green);

	cout << (char)27 << " " << (char)26;
	system("chcp 1251>NUL");
	cout << " " << " - Стрелки выбора, ESC - Отмена или взять карты, ENTER - Подтверждение";
	system("chcp 866>NUL");

	X_bot_start = 57;
	Y_bot_start = 2;
	X_player_start = 57;
	Y_player_start = 49;
	X_step = 11;

	int battlefield_X_start = 40;
	int battlefield_Y_start = 26;
	int battlefield_Y_step = 0;
	int battlefield_X_step = 0;

	bool player_turn_on = false;
	int player_answer_on;

	bool key_to_continue = false;
	bool no_exit = false;

	// Цикл раунда
	do
	{
		//Ход игрока
		if (player_otbilsya)
		{
			SetColor(Red, White);
			SetCursor(8, 37);
			system("chcp 1251>NUL");
			cout << "ХОД ИГРОКА!";
			system("chcp 866>NUL");
			//!!!
			// Процедура проверки возможности подбросить карту
			//!!!
			player_turn_on = Test_Player_turn_on(Battlefield, Player_cards);

			if (!player_turn_on and Cards_in_deck(Battlefield) != card_per_payer * 2)
			{
				//Информационное сообщение
				PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
				SetColor(White, Red);
				SetCursor(62, 15);
				system("chcp 1251>NUL");
				cout << "НЕТ ПОДХОДЯЩИХ КАРТ В РУКЕ! НАЖМИТЕ \"OK\" ДЛЯ ПРОДОЛЖЕНИЯ";
				//cout << "NO REQUIRED CARD IN PLAYER HAND! PRESS \"OK\" TO CONTINUE";
				system("chcp 866>NUL");
				Info_OK((Color)Red, (Color)White, 88, 17);
				PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
			}
			// Проверка максимально возможного числа бодброшенных карт боту
			if (Cards_in_deck(Battlefield) == card_per_payer * 2)
			{
				player_turn_on = false;
				//Информационное сообщение
				PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
				SetColor(White, Red);
				SetCursor(62, 15);
				system("chcp 1251>NUL");
				cout << "ДОСТИГНУТО МАКСИМАЛЬНО ВОЗМОЖНОЕ ЧИСЛО ПОДБРОШЕННЫХ КАРТ";
				system("chcp 866>NUL");
				Info_OK((Color)Red, (Color)White, 88, 17);
				PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
			}

			// Процедура проверки правильности подброски карты
			if (player_turn_on)
			{
				key_to_continue = false;
				no_exit = false;
				do
				{
					no_exit = false;
					temp_i = Chioce_Player_Card(Player_cards, card_ico, fone, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100);
					// Игрок захотел прекратить раунд
					if (temp_i == -1 and Cards_in_deck(Battlefield) > 0)
					{
						//Чтобы не выйти из массива
						temp_i = 0;
						player_turn_on = false;
						break;
					}
					else if (temp_i == -1 and Cards_in_deck(Battlefield) == 0)
					{
						no_exit = true;
					}
					// Проверка возможности подкинуть карту
					if (Cards_in_deck(Battlefield) > 0)
					{
						for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
						{
							if (Battlefield[i].number == Player_cards[temp_i].number)
							{
								key_to_continue = true;
								break;
							}
						}
						if (!key_to_continue)
						{
							PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Blue, 60, 14);
							SetColor(White, Blue);
							SetCursor(62, 15);
							system("chcp 1251>NUL");
							cout << "                 ВЫБРАНА НЕВЕРНАЯ КАРТА!";
							system("chcp 866>NUL");
							//При нажатии rechange возвращает 1
							key_to_continue = 1 - PrintChoice(rechange, 2, 30, 68, 17);
							//Стирает сообщение
							PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
							//Перерисовывает карты игрока
							PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
							Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
							//Игрок говорит Бито!
							if (key_to_continue)
							{
								player_turn_on = false;
							}
						}
					}
					else
					{
						if (!no_exit)
						{
							key_to_continue = true;
							Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
						}
						// Обновление фона в руке игрока
						if (no_exit)
						{
							PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
							Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
						}
					}

				} while (!key_to_continue);
			}
			// Отработка хода
			if (player_turn_on)
			{
				Fill_Pool_Cards(Battlefield, Player_cards, number_cards, temp_i);
				PrintCard(Player_cards[temp_i], card_ico, battlefield_X_start + battlefield_X_step, battlefield_Y_start - battlefield_Y_step, width, height, (Color)LightMagenta, (Color)LightCyan, false);
				NullCard(Player_cards, temp_i);
				Sort_Pool_Cards(Player_cards, number_cards);
				//Очистка фона и перерисовка карт руки игрока
				PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
				Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
				//Функция ответ бота
				botanswer = Bot_Answer_Level_0(Bot_cards, Battlefield);
				if (botanswer != -1)
				{

					PrintCard(Bot_cards[botanswer], card_ico, battlefield_X_start + 5 + battlefield_X_step, battlefield_Y_start - 2 - battlefield_Y_step, width, height, (Color)LightMagenta, (Color)LightCyan, false);
					Fill_Pool_Cards(Battlefield, Bot_cards, number_cards, botanswer);
					NullCard(Bot_cards, botanswer);
					Sort_Pool_Cards(Bot_cards, number_cards);
					PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
					Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
				}
				//Бот взял карты
				else
				{
					//cout << "No CARDS!";
					//Определение номиналов карт на кону
					ReadToTempIntArr(Battlefield, temp_nominal_battlefield_cards);
					//Проверка возможности подбросить карты боту
					// Podbros_To_Bot()
					Podbros_To_Bot(Player_cards, Bot_cards, Battlefield, temp_nominal_battlefield_cards, fone);
					for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
					{
						Fill_Pool_Cards(Bot_cards, Battlefield, number_cards, i);
					}
					NullPoolCards(Battlefield, number_cards);
					Dobor(Cards, Player_cards, Bot_cards, player_otbilsya);
					PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
					Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
					PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
					Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
					//Затирание поля
					PrintCurrentFone(fone, 20, 100, (Color)Green, battlefield_X_start, battlefield_Y_start - 2);
					// Проверка предпоследней карты
					if (Cards_in_deck(Cards) == 1)
					{
						PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
						PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
					}
					// Проверка последней карты
					if (Cards_in_deck(Cards) == 0)
					{
						PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
					}
					//Обновление счетчика карт
					SetCursor(5, 35);
					SetColor(Green, Green);
					cout << "                 " << Cards_in_deck(Cards);
					SetCursor(5, 35);
					SetColor(Red, White);
					system("chcp 1251>NUL");
					cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
					system("chcp 866>NUL");
				}
				battlefield_X_step += 15;
				if (botanswer == -1)
				{
					battlefield_X_step = 0;
					//round = 0;
				}
			}
		}
		//Ход бота
		else
		{
			SetColor(Red, White);
			SetCursor(9, 23);
			system("chcp 1251>NUL");
			cout << "ХОД БОТА!";
			system("chcp 866>NUL");
			//Проверка может ли Бот ходить или подкидывать карты
			player_turn_on = Test_Player_turn_on(Battlefield, Bot_cards);

			//Бот выбирает карту для хода (КОГДА ДОБОР РАВЕН 0 - БОТ МОЖЕТ ХОДИТЬ С КОЗЫРЕЙ ПО МЕНЬШЕМУ НОМИНАЛУ И ПОДКИДЫВАТЬ КОЗЫРИ!)
			temp_i = Bot_Turn_Level_0(Bot_cards, Battlefield, Cards, player_turn_on);
			// Проверка на подбрасывание козырей пока колода добора не пуста
			if (temp_i == -1)
			{
				player_turn_on = false;
			}

			// Проверка максимально возможного числа бодброшенных карт игроку
			if (Cards_in_deck(Battlefield) == card_per_payer * 2)
			{
				player_turn_on = false;
			}

			if (player_turn_on)
			{
				Fill_Pool_Cards(Battlefield, Bot_cards, number_cards, temp_i);
				PrintCard(Bot_cards[temp_i], card_ico, battlefield_X_start + battlefield_X_step, battlefield_Y_start - battlefield_Y_step, width, height, (Color)LightMagenta, (Color)LightCyan, false);
				NullCard(Bot_cards, temp_i);
				Sort_Pool_Cards(Bot_cards, number_cards);
				//Очистка фона и перерисовка карт руки бота
				PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
				Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);

				//Проверка на возможность игрока отбиться
				player_answer_on = Bot_Answer_Level_0(Player_cards, Battlefield);

				// Если игрок может отбиться
				if (player_answer_on != -1)
				{
					do
					{
						if (player_answer_on != -1)
						{
							// Ответ игрока
							temp_i = Chioce_Player_Card(Player_cards, card_ico, fone, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100);
							key_to_continue = false;
							// Игрок хочет взять карту
							if (temp_i == -1)
							{
								PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Blue, 60, 14);
								SetColor(White, Blue);
								SetCursor(62, 15);
								system("chcp 1251>NUL");
								cout << "                 ВЫ ХОТИТЕ ВЗЯТЬ КАРТУ?";
								system("chcp 866>NUL");
								//При нажатии yes PrintChoice возвращает 1
								key_to_continue = 1 - PrintChoice(yesno, 2, 38, 70, 17);
								//Стирает сообщение
								PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
								//Перерисовывает карты игрока
								PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
								Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
								//Игрок говорит БЕРУ!
								if (!key_to_continue)
								{
									player_answer_on = -1;

									//Подброс карт ботом после того как игрок взял карты
									ReadToTempIntArr(Battlefield, temp_nominal_battlefield_cards);
									Podbros_To_Player(Cards, Player_cards, Bot_cards, Battlefield, temp_nominal_battlefield_cards, fone);

									for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
									{
										Fill_Pool_Cards(Player_cards, Battlefield, number_cards, i);
									}

									NullPoolCards(Battlefield, number_cards);
									Dobor(Cards, Player_cards, Bot_cards, player_otbilsya);
									PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
									Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
									PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
									Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
									//Затирание поля
									PrintCurrentFone(fone, 20, 100, (Color)Green, battlefield_X_start, battlefield_Y_start - 2);
									// Проверка предпоследней карты
									if (Cards_in_deck(Cards) == 1)
									{
										PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
										PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
									}
									// Проверка последней карты
									if (Cards_in_deck(Cards) == 0)
									{
										PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
									}
									//Обновление счетчика карт
									SetCursor(5, 35);
									SetColor(Green, Green);
									cout << "                 " << Cards_in_deck(Cards);
									SetCursor(5, 35);
									SetColor(Red, White);
									system("chcp 1251>NUL");
									cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
									system("chcp 866>NUL");
									battlefield_X_step = -15;
									//round = 0;
									break;
								}

							}
							player_answer_on = Player_Answer(Player_cards, Battlefield, temp_i);

							if (!key_to_continue)
							{
								// Если картой можно отбиться
								if (player_answer_on)
								{

									PrintCard(Player_cards[temp_i], card_ico, battlefield_X_start + 5 + battlefield_X_step, battlefield_Y_start - 2 - battlefield_Y_step, width, height, (Color)LightMagenta, (Color)LightCyan, false);
									Fill_Pool_Cards(Battlefield, Player_cards, number_cards, temp_i);
									NullCard(Player_cards, temp_i);
									Sort_Pool_Cards(Player_cards, number_cards);
									PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
									Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
									break;
									//player_answer_on = -1;
								}
								// Если карта ненадлежащей масти чтобы отбиться
								else
								{
									//cout << "Incorrect chioce!!!";
									PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Blue, 60, 14);
									SetColor(White, Blue);
									SetCursor(62, 15);
									system("chcp 1251>NUL");
									cout << "                 ВЫБРАНА НЕВЕРНАЯ КАРТА!";
									system("chcp 866>NUL");
									//При нажатии rechange возвращает 1
									key_to_continue = 1 - PrintChoice(rechange, 2, 30, 68, 17);
									//Стирает сообщение
									PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
									//Перерисовывает карты игрока
									PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
									Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
									//Игрок говорит БЕРУ!
									if (key_to_continue)
									{
										player_answer_on = -1;

										for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
										{
											Fill_Pool_Cards(Player_cards, Battlefield, number_cards, i);
										}
										NullPoolCards(Battlefield, number_cards);
										Dobor(Cards, Player_cards, Bot_cards, player_otbilsya);
										PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
										Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
										PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
										Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
										//Затирание поля
										PrintCurrentFone(fone, 20, 100, (Color)Green, battlefield_X_start, battlefield_Y_start - 2);
										// Проверка предпоследней карты
										if (Cards_in_deck(Cards) == 1)
										{
											PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
											PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
										}
										// Проверка последней карты
										if (Cards_in_deck(Cards) == 0)
										{
											PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
										}
										//Обновление счетчика карт
										SetCursor(5, 35);
										SetColor(Green, Green);
										cout << "                 " << Cards_in_deck(Cards);
										SetCursor(5, 35);
										SetColor(Red, White);
										system("chcp 1251>NUL");
										cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
										system("chcp 866>NUL");

										break;
									}


								}
							}

						}

					} while (player_answer_on != -1);

					if (!key_to_continue)
					{
						player_answer_on = 1;
					}
				}
				//Если игрок не может отбиться и берет карты
				else
				{
					//Информационное сообщение
					PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
					SetColor(White, Red);
					SetCursor(62, 15);
					system("chcp 1251>NUL");
					cout << "НЕТ ПОДХОДЯЩИХ КАРТ В РУКЕ! НАЖМИТЕ \"OK\" ДЛЯ ПРОДОЛЖЕНИЯ";
					//cout << "NO REQUIRED CARD IN PLAYER HAND! PRESS \"OK\" TO CONTINUE";
					system("chcp 866>NUL");
					Info_OK((Color)Red, (Color)White, 88, 17);
					PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
					for (size_t i = 0; i < Cards_in_deck(Battlefield); i++)
					{
						Fill_Pool_Cards(Player_cards, Battlefield, number_cards, i);
					}
					NullPoolCards(Battlefield, number_cards);
					Dobor(Cards, Player_cards, Bot_cards, player_otbilsya);
					PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
					Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
					PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
					Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
					//Затирание поля
					PrintCurrentFone(fone, 20, 100, (Color)Green, battlefield_X_start, battlefield_Y_start - 2);
					// Проверка предпоследней карты
					if (Cards_in_deck(Cards) == 1)
					{
						PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
						PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
					}
					// Проверка последней карты
					if (Cards_in_deck(Cards) == 0)
					{
						PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
					}
					//Обновление счетчика карт
					SetCursor(5, 35);
					SetColor(Green, Green);
					cout << "                 " << Cards_in_deck(Cards);
					SetCursor(5, 35);
					SetColor(Red, White);
					system("chcp 1251>NUL");
					cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
					system("chcp 866>NUL");
				}

				battlefield_X_step += 15;
				if (player_answer_on == -1)
				{
					battlefield_X_step = 0;
					//round = 0;
				}
			}
		}
		//round++;
		
		// Победил бот
		if (Cards_in_deck(Cards) == 0 and Cards_in_deck(Bot_cards) == 0 and Cards_in_deck(Player_cards) != 0)
		{
			//Информационное сообщение Победил бот
			PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
			SetColor(White, Red);
			SetCursor(85, 15);
			system("chcp 1251>NUL");
			cout << "БОТ ПОБЕДИЛ";
			//cout << "BOT WINS!";
			system("chcp 866>NUL");
			Info_OK((Color)Red, (Color)White, 89, 17);
			PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
			//Запись данных в файл
			ReadFileToGamersTable(GameTable, FuncReadF_row_number(temp));
			number_player_games = GameTable[activeuserslot].games;
			number_player_games++;
			GameTable[activeuserslot].games = number_player_games;
			ReWriteFileGamersTable(GameTable, FuncReadF_row_number(temp));
			PrintCurrentFone(fone, fone_row, fone_col, (Color)Green, 1, 1);
			end_con = true;

			NullPoolCards(Cards, number_cards);
			NullPoolCards(Bot_cards, number_cards);
			NullPoolCards(Player_cards, number_cards);
			NullPoolCards(Battlefield, number_cards);
			NullPoolCards(Played_cards, number_cards);
			break;

		}
		// Игрок победил 
		else if (Cards_in_deck(Cards) == 0 and Cards_in_deck(Player_cards) == 0 and Cards_in_deck(Bot_cards) != 0)
		{
			//Информационное сообщение Победил игрок
			PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
			SetColor(White, Red);
			SetCursor(76, 15);
			system("chcp 1251>NUL");
			cout << "ПОЗДРАВЛЯЕМ!!! ВЫ ВЫИГРАЛИ!!!";
			//cout << "CONGRATULATIONS!!! YOU WIN!!!";
			system("chcp 866>NUL");
			Info_OK((Color)Red, (Color)White, 89, 17);
			PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
			//Запись данных в файл
			ReadFileToGamersTable(GameTable, FuncReadF_row_number(temp));
			number_player_games = GameTable[activeuserslot].games;
			number_player_games++;
			wins = GameTable[activeuserslot].wins;
			wins++;
			GameTable[activeuserslot].games = number_player_games;
			GameTable[activeuserslot].wins = wins;
			ReWriteFileGamersTable(GameTable, FuncReadF_row_number(temp));
			PrintCurrentFone(fone, fone_row, fone_col, (Color)Green, 1, 1);
			end_con = true;

			NullPoolCards(Cards, number_cards);
			NullPoolCards(Bot_cards, number_cards);
			NullPoolCards(Player_cards, number_cards);
			NullPoolCards(Battlefield, number_cards);
			NullPoolCards(Played_cards, number_cards);
			break;
		}
		else if (Cards_in_deck(Cards) == 0 and Cards_in_deck(Player_cards) == 0 and Cards_in_deck(Bot_cards) == 0)
		{
			//Информационное сообщение ничья
			PrintCurrentFone(fone, 5, 59, (Color)White, (Color)Red, 60, 14);
			SetColor(White, Red);
			SetCursor(83, 15);
			system("chcp 1251>NUL");
			cout << "!!! НИЧЬЯ !!!";
			//cout << "!!! DROW !!!";
			system("chcp 866>NUL");
			Info_OK((Color)Red, (Color)White, 89, 17);
			PrintCurrentFone(fone, 6, 60, (Color)Green, 60, 14);
			//Запись данных в файл
			ReadFileToGamersTable(GameTable, FuncReadF_row_number(temp));
			number_player_games = GameTable[activeuserslot].games;
			number_player_games++;
			GameTable[activeuserslot].games = number_player_games;
			ReWriteFileGamersTable(GameTable, FuncReadF_row_number(temp));
			PrintCurrentFone(fone, fone_row, fone_col, (Color)Green, 1, 1);
			end_con = true;

			NullPoolCards(Cards, number_cards);
			NullPoolCards(Bot_cards, number_cards);
			NullPoolCards(Player_cards, number_cards);
			NullPoolCards(Battlefield, number_cards);
			NullPoolCards(Played_cards, number_cards);
			break;
		}


		//!!!БЛОК БИТО!!!
		// Условие срабатывания функции бито и добора карт если возможно или требуется.
		if (player_otbilsya)
		{
			temp_number_player_card = Cards_in_deck(Player_cards);
		}
		else
		{
			temp_number_player_card = Cards_in_deck(Bot_cards);
		}
		//if (round == 6 or !player_turn_on)
		if (temp_number_player_card == 0 or !player_turn_on)
		{
			SetCursor(9, 23);
			SetColor(Green, Green);
			cout << "            ";
			SetCursor(8, 37);
			SetColor(Green, Green);
			cout << "            ";
			//round = 0;
			battlefield_X_step = 0;
			//Блок добора
			Dobor(Cards, Player_cards, Bot_cards, player_otbilsya);
			//Очистка фона и перерисовка карт руки игрока
			PrintCurrentFone(fone, 19, 159, (Color)Green, 10, 39);
			Print_players_Cards(Player_cards, card_ico, Cards_in_deck(Player_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) / 100, Y_player_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Player_cards)) % 100, false);
			//Очистка фона и перерисовка карт бота
			PrintCurrentFone(fone, 20, 159, (Color)Green, 10, 1);
			Print_players_Cards(Bot_cards, card_ico, Cards_in_deck(Bot_cards), X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) / 100, Y_bot_start, X_Cards_Coordinat_and_Step(Cards_in_deck(Bot_cards)) % 100, true);
			// Проверка предпоследней карты
			if (Cards_in_deck(Cards) == 1)
			{
				PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
				PrintCard(Cards[35], card_ico, x, y, width, height, (Color)LightMagenta, (Color)LightCyan, false);
			}
			// Проверка последней карты
			if (Cards_in_deck(Cards) == 0)
			{
				PrintCurrentFone(fone, 9, 19, (Color)Green, 4, 25);
			}
			//Функция бито передает ход
			player_otbilsya = bito(Battlefield, Played_cards, player_otbilsya, Cards_in_deck(Battlefield));
			PrintCurrentFone(fone, 12, 100, (Color)Green, battlefield_X_start, battlefield_Y_start - 2);
			Print_Played_Card(Played_cards, card_ico);
			SetCursor(5, 35);
			SetColor(Green, Green);
			cout << "                 " << Cards_in_deck(Cards);
			SetCursor(5, 35);
			SetColor(Red, White);
			system("chcp 1251>NUL");
			cout << "КАРТ В КОЛОДЕ: " << Cards_in_deck(Cards);
			system("chcp 866>NUL");
		}
	} while (!end_con);

	delete temp_nominal_battlefield_cards;
}


//!!!!!!!!!!!!
// Game func end
//!!!!!!!!!!!!

int main()
{
	if (presentation_pause)
	{
		system("pause>>NULL");
	}

	//setlocale(LC_ALL, "rus");
	//SetConsoleCP(1251);
	//SetConsoleOutputCP(1251);
	srand(time(NULL));
	system("mode con cols=180 lines=60");
	//Скрытие курсора
	CONSOLE_CURSOR_INFO structCursorInfo;
	GetConsoleCursorInfo(hStdOut, &structCursorInfo);
	structCursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hStdOut, &structCursorInfo);

	//Размер фона игрового меню
	int row = 60;
	int col = 180;
	char** fone = new char* [row];
	for (size_t i = 0; i < row; i++)
	{
		fone[i] = new char[col];
	}
	//Заплатка игрового меню
	int zrow = 58;//12//25
	int zcol = 178;//31///55
	char** clsfone = new char* [zrow];
	for (size_t i = 0; i < zrow; i++)
	{
		clsfone[i] = new char[zcol];
	}
	char** players_menu = new char* [zrow];
	for (size_t i = 0; i < zrow; i++)
	{
		players_menu[i] = new char[zcol];
	}
	//Настройки главного меню
	const int menustrings = 3;
	const int yes_no = 2;

	system("chcp 1251>NUL");
	string menu[menustrings]{ "НОВАЯ ИГРА", "ИГРОКИ", "ВЫХОД" };
	string choice[yes_no]{ "ДА", "НЕТ" };
	string editplayersmenu[menustrings]{ "СОЗДАТЬ ИГРОКА", "УДАЛИТЬ ИГРОКА", "ВЫХОД ИЗ МЕНЮ" };
	system("chcp 866>NUL");

	//string menu[menustrings]{ "NEW GAME", "PLAYERS", "QUIT" };
	//string choice[yes_no]{ "YES", "NO" };
	//string editplayersmenu[menustrings]{ "CREATE PLAYER", "DELETE PLAYER", "EXIT MENU" };

	// Переменная принимающая результат функции главного меню
	int activestring = -1;
	//bool chioce;
	bool startgame = false;

	//Даннные об игроках
	FILE* pf;
	int bufersizeplayers = 12;
	int file_size = -1;
	Gamers* GamersTable = new Gamers[bufersizeplayers];

	string currentusername;
	string currentuserpassword;
	//currentuserslot для удаления пользователя
	int currentuserslot;
	string password_atht;
	int activeuserslot = 0;
	bool passwordvalidation = false;

	// Колода карт
	int cards_number = 36;
	int mast = 4;
	int cards_per_player = 6;
	Koloda* Cards = new Koloda[cards_number];
	int card_nominal = 9;
	Koloda* Bot_Cards = new Koloda[cards_number];
	Koloda* Player_Cards = new Koloda[cards_number];
	Koloda* Played_Cards = new Koloda[cards_number];
	Koloda* Battlefield = new Koloda[cards_number];

	int card_width = 9;
	int card_height = 9;
	char** card_ico = new char* [card_height];
	for (size_t i = 0; i < card_height; i++)
	{
		card_ico[i] = new char[card_width];
	}

	Nullfone(fone, row, col);
	PrintFon(fone, row, col);

	//Интро
	if (intro)
	{
		KolodaGenerator(Cards, mast, cards_number, card_nominal);
		Print_Game_durak(Red, Green);
		KolodaMix(Cards, cards_number, 100500);
		Print_36_cards_on_table(Cards, card_ico, card_width, card_height, cards_number, card_nominal);
		Print_Game_durak(Green, Green, false);
	}

	//Проверка на отсутствие 5 карт одинаковой масти в руке одного из игроков и присутствия хотябы одного козыря у игроков
	bool three_cards_different_bot_hand = false;
	bool three_cards_different_player_hand = false;
	bool one_card_is_trump_in_any_hand = false;
	//long unsigned int rounds = 0;
	//Основной цикл
	do
	{
		//Цикл раздачи карт
		do
		{
			// Обнуление рук бота, игрока и битых карт
			NullPoolCards(Bot_Cards, cards_number);
			NullPoolCards(Player_Cards, cards_number);
			NullPoolCards(Played_Cards, cards_number);
			NullPoolCards(Battlefield, cards_number);
			three_cards_different_bot_hand = false;
			three_cards_different_player_hand = false;
			one_card_is_trump_in_any_hand = false;

			// Генерация колоды, перетасовывание и объявление козыря 
			KolodaGenerator(Cards, mast, cards_number, card_nominal);
			KolodaMix(Cards, cards_number, 100500);
			Trump_13(Cards, cards_per_player * 2, cards_number);

			// Раздача карт
			RazdachaCards(Cards, Player_Cards, Bot_Cards, cards_number, cards_per_player);
			SwapTrump(Cards, cards_per_player * 2, cards_number - 1);

			//Проверка на три разные масти карт в руке бота
			int is_hearts = 0;
			int is_diamonds = 0;
			int is_clubs = 0;
			int is_spades = 0;

			for (size_t i = 0; i < cards_per_player; i++)
			{
				if (Bot_Cards[i].suit == 3)
				{
					is_hearts = 1;
				}
				if (Bot_Cards[i].suit == 4)
				{
					is_diamonds = 1;
				}
				if (Bot_Cards[i].suit == 5)
				{
					is_clubs = 1;
				}
				if (Bot_Cards[i].suit == 6)
				{
					is_spades = 1;
				}
			}
			if (is_hearts + is_diamonds + is_clubs + is_spades >= 3)
			{
				three_cards_different_bot_hand = 1;
			}

			//Проверка на три разные масти карт в руке игрока
			is_hearts = 0;
			is_diamonds = 0;
			is_clubs = 0;
			is_spades = 0;

			for (size_t i = 0; i < cards_per_player; i++)
			{
				if (Player_Cards[i].suit == 3)
				{
					is_hearts = 1;
				}
				if (Player_Cards[i].suit == 4)
				{
					is_diamonds = 1;
				}
				if (Player_Cards[i].suit == 5)
				{
					is_clubs = 1;
				}
				if (Player_Cards[i].suit == 6)
				{
					is_spades = 1;
				}
			}
			if (is_hearts + is_diamonds + is_clubs + is_spades >= 3)
			{
				three_cards_different_player_hand = 1;
			}

			// Проверка на наличие хотябы одного козыря в руках игроков
			int number_trumps = 0;
			for (size_t i = 0; i < cards_number; i++)
			{
				if (Cards[i].trump == true)
				{
					number_trumps++;
				}
			}
			if (number_trumps < 9)
			{
				one_card_is_trump_in_any_hand = 1;
			}
			//rounds++;
		} while (three_cards_different_bot_hand + three_cards_different_player_hand + one_card_is_trump_in_any_hand < 3);
		
		// //Количество попыток раздать карты
		//system("chcp 1251>NUL");
		//cout << " Количество попыток раздать карты: " << rounds << endl;
		//system("chcp 866>NUL");
		//system("pause>>NULL");


		if (!startgame)
		{

			activestring = SelectMenuItem(menu, menustrings);
		}
		errno_t file_error = fopen_s(&pf, "PlayersSuperSecureData.txt", "r");
		if (file_error == 0)
		{
			file_size = _filelength(_fileno(pf));
			fclose(pf);
		}

		// Новая игра
		if (activestring == 0)
		{
			PrintCurrentFone(clsfone, 25, 55, (Color)Green, 65, 20);

			// Если нет игроков
			if (file_error != 0 or file_size == 0)
			{
				zrow = 8;
				zcol = 50;
				PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Blue, 65, 22);
				SetColor(White, Black);

				SetColor(White, Red);
				SetCursor(81, 24);
				system("chcp 1251>NUL");
				cout << "НЕТ ИГРОКОВ В СПИСКЕ!";
				SetCursor(74, 25);
				cout << "ВЫ ХОТИТЕ СОЗДАТЬ НОВОГО ИГРОКА?";
				SetColor(White, Black);
				boolchioce = PrintChoice(choice, yes_no, 20, 80, 27);
				system("chcp 866>NUL");
				if (boolchioce)
				{
					startgame = true;
					activestring = -1;
					zrow = 12;
					zcol = 52;
					boolchioce = false;
					PrintCurrentFone(clsfone, 25, 53, (Color)Green, 65, 15);
					PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Blue, 65, 22);
					SetColor(White, Blue);
					SetCursor(82, 24);
					system("chcp 1251>NUL");
					cout << "ВВЕДИТЕ ИМЯ ИГРОКА:";
					system("chcp 866>NUL");
					//cout << "ENTER PLAYER NAME:";
					PrintCurrentFone(players_menu, 3, 42, (Color)White, (Color)Blue, 70, 27, false);
					PrintCurrentFone(players_menu, 1, 40, (Color)Black, 71, 28);
					SetColor(White, Black);
					SetCursor(71, 28);
					currentusername = Login_Password();
					//cin >> currentusername;
					PrintCurrentFone(players_menu, 1, 40, (Color)Black, 71, 28);
					SetColor(White, Blue);
					SetCursor(82, 24);
					system("chcp 1251>NUL");
					cout << "  ВВЕДИТЕ ПАРОЛЬ:    ";
					system("chcp 866>NUL");
					//cout << "ENTER PLAYER PASSWORD:";
					SetColor(White, Black);
					SetCursor(71, 28);
					currentuserpassword = Login_Password('P');
					//cin >> currentuserpassword;
					if (file_size != 0)
					{
						fopen_s(&pf, "PlayersSuperSecureData.txt", "w");
					}
					fclose(pf);
					AddDataToGamersTable(GamersTable, currentusername, currentuserpassword);
					AddGamersTableRowDataToFile(GamersTable->user_id, GamersTable->name, GamersTable->password, GamersTable->games, GamersTable->wins);
					PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Blue, 65, 22);
					activeuserslot = 0;
					SetColor(White, Blue);
					SetCursor(85, 24);
					system("chcp 1251>NUL");
					cout << "ПОЗДРАВЛЯЕМ !";
					SetCursor(83, 25);
					cout << "ИГРОК БЫЛ СОЗДАН!";
					//cout << "CONGRATULATIONS!";
					//SetCursor(79, 25);
					//cout << "PLAYER HAS BEEN CREATED!";
					SetCursor(82, 27);
					cout << "ХОТИТЕ НАЧАТЬ ИГРУ?";
					system("chcp 866>NUL");


					boolchioce = PrintChoice(choice, yes_no, 20, 80, 30);
					if (boolchioce)
					{
						// Создан первый игрок. Запускается функция игры 
						GameFunc(Cards, Player_Cards, Bot_Cards, Played_Cards, Battlefield, GamersTable, cards_number, cards_per_player, card_ico, 9, 25, card_width, card_height, activeuserslot, clsfone, 58, 178);
						boolchioce = false;
						startgame = false;
					}
					// Если пользователь создал первый аккаунт, но отказался от игры
					else
					{
						startgame = false;
						zrow = 25;
						zcol = 53;
						PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 15);
					}

				}
				// Если пользователь отказался создавать первый аккаунт и отказался от игры
				else
				{
					zrow = 25;
					zcol = 53;
					PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 15);
				}
			}
			// Если файл с аккаунтами прочитан и в нем есть данные об игроках запускается меню выбора игрока
			else
			{
				zrow = 24;
				zcol = 53;
				PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 15);
				PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Magenta, 65, 15);
				SetColor(White, Magenta);

				SetCursor(84, 17);
				system("chcp 1251>NUL");
				cout << "ВЫБЕРИТЕ ИГРОКА:";
				SetCursor(70, 19);
				/*cout << 'N'<<(char)248;*/
				cout << "СЛОТ";
				SetCursor(79, 19);
				cout << "ИГРОК";
				SetCursor(96, 19);
				cout << "ИГРЫ";
				SetCursor(109, 19);
				cout << "ПОБЕДЫ";
				system("chcp 866>NUL");
				//SetCursor(84, 17);
				//cout << "CHOOSE A PLAYER:";
				//SetCursor(70, 19);
				///*cout << 'N'<<(char)248;*/
				//cout << "SLOT";
				//SetCursor(79, 19);
				//cout << "PLAYER";
				//SetCursor(96, 19);
				//cout << "GAMES";
				//SetCursor(109, 19);
				//cout << "WINS";
				SetCursor(71, 21);
				activeuserslot = UserChoice(GamersTable, FuncReadF_row_number(temp), (Color)White, (Color)Magenta);
				// Если пользователь выбрал игрока - проверка пароля
				if (activeuserslot != -1)
				{
					//bool chioce = true;
					do
					{
						do
						{
							//Проверка пароля
							zrow = 9;
							zcol = 40;
							PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Blue, 71, 22);
							SetColor(White, Blue);
							SetCursor(84, 23);
							system("chcp 1251>NUL");
							cout << "ВВЕДИТЕ ПАРОЛЬ:";
							system("chcp 866>NUL");
							PrintCurrentFone(players_menu, 3, 32, (Color)White, (Color)Blue, 75, 24, false);
							PrintCurrentFone(players_menu, 1, 30, (Color)Black, 76, 25);
							SetColor(White, Black);
							SetCursor(76, 25);
							password_atht = Login_Password('P');
							//cin >> password_atht;
							passwordvalidation = PasswordCheck(GamersTable, password_atht, activeuserslot);
							if (passwordvalidation == true)
							{
								SetColor(White, Green);
								SetCursor(83, 27);
								system("chcp 1251>NUL");
								cout << " ПАРОЛЬ ПРИНЯТ ";
								system("chcp 866>NUL");
								SetColor(White, Black);
								Sleep(sleep1000);
								// Игрок выбран из списка. Запускается функция игры 
								GameFunc(Cards, Player_Cards, Bot_Cards, Played_Cards, Battlefield, GamersTable, cards_number, cards_per_player, card_ico, 9, 25, card_width, card_height, activeuserslot, clsfone, 58, 178);
								boolchioce = false;
								startgame = false;
							}
							else
							{
								SetColor(White, Red);
								SetCursor(75, 27);
								system("chcp 1251>NUL");
								cout << " ДОСТУП ЗАПРЕЩЕН! НЕВЕРНЫЙ ПАРОЛЬ" << endl;
								SetCursor(82, 28);
								SetColor(White, Blue);
								cout << "ПОПРОБУЕТЕ ЕЩЕ РАЗ?";
								system("chcp 866>NUL");
								SetColor(White, Black);
								boolchioce = PrintChoice(choice, yes_no, 20, 80, 29);

								activestring = -1;

							}
						} while (boolchioce);
						startgame = false;
						zrow = 25;
						zcol = 54;
						PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 15);

					} while (startgame);
				}
				else
				{
					zrow = 25;
					zcol = 54;
					PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 15);
				}
			}
			SetColor(White, Black);
		}
		// Меню игроков
		if (activestring == 1)
		{

			int choisekey = -1;
			do
			{
				errno_t file_error = fopen_s(&pf, "PlayersSuperSecureData.txt", "r");
				if (file_error == 0)
				{
					file_size = _filelength(_fileno(pf));
					fclose(pf);
				}
				zrow = 24;
				zcol = 53;
				//PrintCurrentFone(clsfone, zrow, zcol, (Color)Green, 65, 20);
				PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Magenta, 65, 15);
				SetColor(White, Magenta);
				SetCursor(80, 17);
				system("chcp 1251>NUL");
				cout << "ЗАРЕГИСТРИРОВАННЫЕ ИГРОКИ:";
				SetCursor(70, 19);
				/*cout << 'N'<<(char)248;*/
				cout << "СЛОТ";
				SetCursor(79, 19);
				cout << "ИГРОК";
				SetCursor(96, 19);
				cout << "ИГРЫ";
				SetCursor(109, 19);
				cout << "ПОБЕДЫ";
				system("chcp 866>NUL");
				if (file_error != 0 or file_size == 0)
				{
					if (file_size != 0)
					{
						fopen_s(&pf, "PlayersSuperSecureData.txt", "w");
					}
					fclose(pf);
					NewPlayerWindow(players_menu, 0, 0, (Color)Green, (Color)White, (Color)Blue, 8, 41, 65, 20, 71, 23);
					SetColor(White, Red);
					SetCursor(82, 25);
					system("chcp 1251>NUL");
					cout << "НЕТ ИГРОКОВ В СПИСКЕ!";
					SetCursor(76, 26);
					cout << "ВЫ ХОТИТЕ СОЗДАТЬ НОВОГО ИГРОКА?";
					SetColor(White, Black);
					boolchioce = PrintChoice(choice, yes_no, 20, 80, 28);
					system("chcp 866>NUL");

					// Запись первого игрока
					if (boolchioce)
					{
						SetColor(White, Blue);
						SetCursor(82, 25);
						cout << "                     ";
						SetCursor(76, 26);
						cout << "                                ";
						SetCursor(82, 26);
						system("chcp 1251>NUL");
						cout << "ВВЕДИТЕ ИМЯ ИГРОКА:";
						system("chcp 866>NUL");
						SetCursor(71, 28);
						PrintCurrentFone(players_menu, 3, 31, (Color)White, (Color)Blue, 76, 27, false);
						PrintCurrentFone(players_menu, 1, 29, (Color)Black, 77, 28);
						SetCursor(77, 28);
						currentusername = Login_Password();
						//cin >> currentusername;
						SetColor(White, Blue);
						SetCursor(82, 26);
						cout << "                    ";
						SetCursor(84, 26);
						system("chcp 1251>NUL");
						cout << "ВВЕДИТЕ ПАРОЛЬ:  ";
						system("chcp 866>NUL");
						SetCursor(71, 28);
						PrintCurrentFone(players_menu, 3, 31, (Color)White, (Color)Blue, 76, 27, false);
						PrintCurrentFone(players_menu, 1, 29, (Color)Black, 77, 28);
						SetCursor(77, 28);
						currentuserpassword = Login_Password('P');
						//cin >> currentuserpassword;
						PrintCurrentFone(players_menu, 4, 31, (Color)Blue, 76, 26);
						SetColor(White, Blue);
						SetCursor(84, 27);
						system("chcp 1251>NUL");
						cout << "ПОЗДРАВЛЯЕМ!";
						SetCursor(81, 28);
						cout << "ИГРОК БЫЛ СОЗДАН!";
						system("chcp 866>NUL");
						AddDataToGamersTable(GamersTable, currentusername, currentuserpassword);
						AddGamersTableRowDataToFile(GamersTable->user_id, GamersTable->name, GamersTable->password, GamersTable->games, GamersTable->wins);
						Sleep(sleep1000);
						PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Magenta, 65, 15);
						SetColor(White, Magenta);
						SetCursor(80, 17);
						system("chcp 1251>NUL");
						cout << "ЗАРЕГИСТРИРОВАННЫЕ ИГРОКИ:";
						SetCursor(70, 19);
						cout << "СЛОТ";
						SetCursor(79, 19);
						cout << "ИГРОК";
						SetCursor(96, 19);
						cout << "ИГРЫ";
						SetCursor(109, 19);
						cout << "ПОБЕДЫ";
						system("chcp 866>NUL");
					}
					else
					{
						PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Magenta, 65, 15);
						SetColor(White, Magenta);
						SetCursor(80, 17);
						system("chcp 1251>NUL");
						cout << "ЗАРЕГИСТРИРОВАННЫЕ ИГРОКИ:";
						SetCursor(70, 19);
						cout << "СЛОТ";
						SetCursor(79, 19);
						cout << "ИГРОК";
						SetCursor(96, 19);
						cout << "ИГРЫ";
						SetCursor(109, 19);
						cout << "ПОБЕДЫ";
						system("chcp 866>NUL");
					}
				}
				// Запись данных из файла в структуру
				ReadFileToGamersTable(GamersTable, FuncReadF_row_number(temp));
				ShowGamersTable(GamersTable, FuncReadF_row_number(temp), (Color)White, (Color)Magenta, 70, 21, 9, 26, 39);
				choisekey = PrintChoice(editplayersmenu, menustrings, (Color)Red, (Color)White, 17, 68, 37);

				// Создание игрока и проверка на предельное количество игроков
				if (choisekey == 0 and FuncReadF_row_number(temp) < maxplayerslot)
				{
					NewPlayerWindow(players_menu, 0, 0, (Color)Green, (Color)White, (Color)Blue, 8, 41, 65, 20, 71, 23);
					SetColor(White, Blue);
					SetCursor(82, 26);
					system("chcp 1251>NUL");
					cout << "ВВЕДИТЕ ИМЯ ИГРОКА:";
					system("chcp 866>NUL");
					SetCursor(71, 28);
					PrintCurrentFone(players_menu, 3, 31, (Color)White, (Color)Blue, 76, 27, false);
					PrintCurrentFone(players_menu, 1, 29, (Color)Black, 77, 28);
					SetCursor(77, 28);
					currentusername = Login_Password();
					//cin >> currentusername;
					// Проверка на уже существующего пользователя
					bool nocurrentuser = true;
					for (size_t i = 0; i < FuncReadF_row_number(temp); i++)
					{
						if (GamersTable[i].name == currentusername)
						{
							nocurrentuser = false;
						}
					}
					if (nocurrentuser)
					{
						SetColor(White, Blue);
						SetCursor(82, 26);
						cout << "                  ";
						SetCursor(84, 26);
						system("chcp 1251>NUL");
						cout << "ВВЕДИТЕ ПАРОЛЬ:  ";
						system("chcp 866>NUL");
						SetCursor(71, 28);
						PrintCurrentFone(players_menu, 3, 31, (Color)White, (Color)Blue, 76, 27, false);
						PrintCurrentFone(players_menu, 1, 29, (Color)Black, 77, 28);
						SetCursor(77, 28);
						currentuserpassword = Login_Password('P');
						//cin >> currentuserpassword;
						PrintCurrentFone(players_menu, 4, 31, (Color)Blue, 76, 26);
						AddDataToGamersTableRowZ(GamersTable, currentusername, currentuserpassword, FuncReadF_row_number(temp));
						AddGamersTableRowDataToFile(GamersTable[FuncReadF_row_number(temp)].user_id, GamersTable[FuncReadF_row_number(temp)].name, GamersTable[FuncReadF_row_number(temp)].password, GamersTable[FuncReadF_row_number(temp)].games, GamersTable[FuncReadF_row_number(temp)].wins);
						SetColor(White, Blue);
						SetCursor(86, 26);
						system("chcp 1251>NUL");
						cout << "ПОЗДРАВЛЯЕМ!";
						SetCursor(83, 27);
						cout << "ИГРОК БЫЛ СОЗДАН!";
						system("chcp 866>NUL");
						Sleep(sleep1000);
					}
					else
					{
						PrintCurrentFone(players_menu, 4, 53, (Color)White, (Color)Red, 65, 10);
						SetColor(White, Red);
						SetCursor(74, 11);
						system("chcp 1251>NUL");
						cout << "ИГРОК УЖЕ СОЗДАН! ВВЕДИТЕ ДРУГОЕ ИМЯ";
						system("chcp 866>NUL");
						Info_OK((Color)Red, (Color)White, 91, 12);
						PrintCurrentFone(players_menu, 5, 54, (Color)Green, 65, 10);
					}

					//PrintCurrentFone(players_menu, zrow, zcol, (Color)White, (Color)Magenta, 65, 15);
					//SetColor(White, Magenta);
					//SetCursor(84, 17);
					//cout << "REGISTERED PLYERS:";
					//SetCursor(70, 19);
					//cout << "SLOT";
					//SetCursor(79, 19);
					//cout << "PLAYER";
					//SetCursor(96, 19);
					//cout << "GAMES";
					//SetCursor(109, 19);
					//cout << "WINS";
				}
				else if (FuncReadF_row_number(temp) >= maxplayerslot and choisekey != 1 and choisekey != 2)
				{
					PrintCurrentFone(players_menu, 4, 53, (Color)White, (Color)Red, 65, 10);
					SetColor(White, Red);
					SetCursor(67, 11);
					system("chcp 1251>NUL");
					cout << "ВСЕ СЛОТЫ ИСПОЛЬЗУЮТСЯ. УДАЛИТЕ ОДНОГО ИЗ ИГРОКОВ";
					//cout << "ALL SLOTS ARE USED. REMOVE ONE OF THE PLAYERS";
					system("chcp 866>NUL");
					Info_OK((Color)Red, (Color)White, 91, 12);
					PrintCurrentFone(players_menu, 5, 54, (Color)Green, 65, 10);
				}

				// Удаление пользователя
				if (choisekey == 1)
				{
					PrintCurrentFone(players_menu, 6, 53, (Color)White, (Color)Red, 65, 8);
					SetColor(White, Red);
					SetCursor(72, 9);
					system("chcp 1251>NUL");
					cout << "ВВЕДИТЕ НОМЕР СЛОТА ДЛЯ УДАЛЕНИЯ ИГРОКА";
					//cout << "ENTER SLOT NUMBER TO REMOVE A PLAYER";
					system("chcp 866>NUL");
					//Info_OK((Color)Red, (Color)White, 91, 12);
					PrintCurrentFone(players_menu, 3, 31, (Color)White, (Color)Red, 76, 10, false);
					PrintCurrentFone(players_menu, 1, 29, (Color)Black, 77, 11);
					SetCursor(77, 11);
					cin >> currentuserslot;
					if (currentuserslot > 0 and currentuserslot <= FuncReadF_row_number(temp))
					{
						ReWriteFileGamersTable(GamersTable, FuncReadF_row_number(temp), currentuserslot - 1);
					}
					else
					{
						PrintCurrentFone(players_menu, 6, 53, (Color)White, (Color)Red, 65, 8);
						SetColor(White, Red);
						SetCursor(72, 9);
						system("chcp 1251>NUL");
						cout << "                                       ";
						//cout << "ENTER SLOT NUMBER TO REMOVE A PLAYER";
						SetCursor(72, 10);
						cout << "НЕВЕРНЫЙ НОМЕР СЛОТА! ПОПРОБУЙТЕ СНОВА";
						//cout << "INCORRECT SLOT NUMBER! TRY AGAIN";
						system("chcp 866>NUL");
						Info_OK((Color)Red, (Color)White, 91, 12);
					}
					PrintCurrentFone(players_menu, 7, 54, (Color)Green, 65, 8);
				}

				//Выход
				if (choisekey == 2)
				{
					SetCursor(71, 28);
					PrintCurrentFone(clsfone, 25, 54, (Color)Green, 65, 15);
				}
			} while (choisekey != 2);
		}

	} while (activestring != 2);
	system("cls");

	Delete_D_Arr(clsfone, zrow);
	Delete_D_Arr(players_menu, zrow);
	//system("pause>>NULL");
	return 0;
}