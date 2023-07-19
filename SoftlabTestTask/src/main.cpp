#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

#include <parser.h>

void PrintHelp(const std::wstring& exeName)
{
	std::wcout << L"Некорректные параметры. Пример использования:" << std::endl;
	std::wcout << exeName << L" <Path to input> <Path to output>" << std::endl;
}

// Used wmain instead the `main` to support the multi-language paths
int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "Russian");
#ifdef _WIN32
	SetConsoleOutputCP(1251);
#endif

	if (argc != 3)
	{
		PrintHelp(argv[0]);
		return 0;
	}

	std::filesystem::path inputFilepath = argv[1];
	std::filesystem::path outputFilepath = argv[2];

	if (!std::filesystem::is_regular_file(inputFilepath))
	{
		PrintHelp(argv[0]);
		return 0;
	}

	auto parser = Parser();
	auto node = parser.Parse(inputFilepath);
	if (!node)
	{
		std::wcout << L"Неверный формат данных" << std::endl;
		return 0;
	}
	
	if (outputFilepath.has_parent_path())
		std::filesystem::create_directories(outputFilepath.parent_path());

	std::ofstream outputStream(outputFilepath, std::ios::out);
	if (!outputStream.is_open())
	{
		std::wcout << L"Не удалось провести запись в выходной файл." << std::endl;
		return 0;
	}
	
	node_util::PrintNode(&outputStream, node.value());
	std::wcout << L"Задача успешно выполнена. Результат записан в файл: " << outputFilepath << std::endl;
}
