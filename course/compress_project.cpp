#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

#define LONG 10000000000

using namespace std;

class BaseCompress
{
protected:
	char* ptr;
	int len;
	int capacity;
public:
	BaseCompress(int Capacity = 256)
	{
		len = 0;
		capacity = Capacity;
		ptr = new char[capacity];
	}
	BaseCompress(string s)
	{
		len = s.length();
		capacity = len >= 256 ? len + 1 : 256;
		ptr = new char[capacity];
		for (int i = 0; i < len; i++)
			ptr[i] = s[i];
		ptr[len] = '\0';
	}
	~BaseCompress()
	{
		if (ptr != NULL)
			delete[] ptr;
		len = 0;
	}
	virtual string compress() = 0;
	virtual string decompress() = 0;
	virtual void showStats() = 0;
};

class RLE : public BaseCompress
{
public:
	RLE(int Capacity = 256) : BaseCompress(Capacity) {}
	RLE(string s) : BaseCompress(s) {}
	~RLE() {}
	string compress()
	{
		string* dict = new string[capacity];
		string output;
		int count = 0, num = 1;

		for (int i = 0; i < len - 1; i++)
		{
			if (ptr[i] == ptr[i + 1])
				num++;
			else
			{
				dict[count] = to_string(num);
				count++;
				dict[count] = ptr[i];
				count++;
				num = 1;
			}
		}

		if (ptr[len - 2] == ptr[len - 1])
			dict[count] = to_string(count);
		else
			dict[count] = "1";
		count++;
		dict[count] = ptr[len - 1];
		count++;

		ofstream file("stats.txt");
		file << "1\n";
		for (int i = 0; i < count; i++)
		{
			if (i % 2 != 0)
				file << dict[i] << " ";
			output += dict[i];
		}
		file << '\n';
		for (int i = 0; i < count; i++)
		{
			if (i % 2 == 0)
				file << " " << dict[i];
		}
		file.close();

		delete[] dict;
		dict = NULL;
		return output;
	}
	string decompress()
	{
		string output;
		int count = 0;
		char c;

		for (int i = 0; i < len - 1; i++)
		{
			if (ptr[i] >= '0' && ptr[i] <= '9')
			{
				int n = ptr[i] - '0';
				for (int j = i + 1; j < len; j++)
				{
					if (ptr[j] >= '0' && ptr[j] <= '9')
						n = n * 10 + (ptr[j] - '0');
					else
					{
						c = ptr[j];
						break;
					}
				}
				for (int j = 0; j < n; j++)
					output += c;
				count += n;
			}
		}
		return output;
	}
	void showStats()
	{
		ifstream file("stats.txt");
		{
			if (file.is_open())
			{
				char c;
				file.get(c);
				if (c == '1')
				{
					string line;
					getline(file, line);
					char* sym = new char[capacity];
					int* num = new int[capacity];
					int count_int = 0, count_str = 0;
					bool flag = false;
					line = "";

					while (!file.eof())
					{
						if (count_str < capacity - 1 && count_int < capacity - 1)
						{
							getline(file, line, ' ');
							if (line != "\n")
							{
								if (flag)
								{
									num[count_int] = stoi(line);
									count_int++;
								}
								else
								{
									sym[count_str] = line[0];
									count_str++;
								}
							}
							else
								flag = true;
						}
					}
					file.close();

					cout << "\nCoding method: RLE Algorithm\n";
					cout << "\nCompression Data: \n";
					cout << " Symbol | Repeats " << endl;
					cout << "------------------" << endl;
					for (int i = 0; i < count_str; i++)
						cout << " " << sym[i] << " | " << num[i] << endl;

					delete[] sym;
					sym = NULL;
					delete[] num;
					num = NULL;
				}
			}
			else
			{
				cout << "\nFile 'stats.txt' was corrupted!" << endl;
				exit(1);
			}
		}
	}
};

class Arithmetic : public BaseCompress
{
public:
	Arithmetic(int Capacity = 256) : BaseCompress(Capacity) {}
	Arithmetic(string s) : BaseCompress(s) {}
	~Arithmetic() {}
	string compress()
	{
		double* temp = new double[capacity];
		double* period = new double[capacity];
		char* sym = new char[capacity];
		int count = 0, len_res = 0;
		double left = 0, right = 1, line = 1, save = 0, left_cpy = 0;
		bool flag = true;

		if (len > 10)
			cout << "\nAttention: possible data loss!\n";

		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < len_res; j++)
			{
				if (ptr[i] == sym[j])
				{
					flag = false;
					break;
				}
			}
			if (flag)
			{
				len_res++;
				sym[len_res - 1] = ptr[i];
			}
			flag = true;
		}

		for (int i = 0; i < len_res; i++)
		{
			for (int j = 0; j < len; j++)
			{
				if (sym[i] == ptr[j])
					count++;
			}
			period[i] = ((double)count) / len + save;
			save = period[i];
			temp[i] = period[i];
			count = 0;
		}

		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < len_res; j++)
			{
				if (ptr[i] == sym[j])
				{
					if (j == 0)
					{
						left = left_cpy;
						right = temp[j];
					}
					else
					{
						left = temp[j - 1];
						right = temp[j];
					}

					line = right - left;
					left_cpy = left;
					for (int k = 0; k < len_res; k++)
						temp[k] = left + period[k] * line;
				}
			}
		}

		ofstream file("stats.txt");
		file << "2\n" << len << "\n";
		for (int i = 0; i < len_res; i++)
			file << sym[i] << " ";
		file << '\n';
		for (int i = 0; i < len_res - 1; i++)
			file << " " << (long long)(period[i] * LONG);
		file.close();

		string output;
		output = to_string((long long)((left + right) / 2 * LONG));
		delete[] temp;
		temp = NULL;
		delete[] period;
		period = NULL;
		delete[] sym;
		sym = NULL;
		return output;
	}
	string decompress()
	{
		char* sym = new char[capacity];
		double* period = new double[capacity];

		double res = atof(ptr) / LONG;
		string output, line;
		int res_count = 0, count_doub = 0, count_str = 0;
		char c;
		bool flag = false;

		ifstream file("stats.txt");
		if (file.is_open())
		{
			getline(file, line); getline(file, line);
			res_count = stoi(line);
			while (!file.eof())
			{
				if (count_str < capacity - 1 && count_doub < capacity - 1)
				{
					getline(file, line, ' ');
					if (line != "\n")
					{
						if (flag)
						{
							period[count_doub] = atof(line.c_str()) / LONG;
							count_doub++;
						}
						else
						{
							sym[count_str] = line[0];
							count_str++;
						}
					}
					else
						flag = true;
				}
			}
			file.close();
			period[count_doub] = 1;
			count_doub++;
		}
		else
		{
			cout << "File 'compressed.txt' was corrupted!" << endl;
			exit(1);
		}

		for (int i = 0; i < res_count; i++)
		{
			for (int j = 0; j < count_str; j++)
			{
				if (res < period[j])
				{
					output += sym[j];
					if (j == 0)
						res /= period[j];
					else
						res = (res - period[j - 1]) / (period[j] - period[j - 1]);
					break;
				}

			}
		}

		delete[] sym;
		sym = NULL;
		delete[] period;
		period = NULL;
		return output;
	}
	void showStats()
	{
		ifstream file("stats.txt");
		if (file.is_open())
		{
			char c;
			file.get(c);
			if (c == '2')
			{
				string line;
				getline(file, line); getline(file, line);
				char* sym = new char[capacity];
				double* period = new double[capacity];
				int count_int = 0, count_str = 0;
				line = "";
				bool flag = false;

				while (!file.eof())
				{
					if (count_str < capacity - 1 && count_int < capacity - 1)
					{
						getline(file, line, ' ');
						if (line != "\n")
						{
							if (flag)
							{
								period[count_int] = atof(line.c_str()) / LONG;
								count_int++;
							}
							else
							{
								sym[count_str] = line[0];
								count_str++;
							}
						}
						else
							flag = true;
					}
				}
				period[count_int] = 1;
				count_int++;
				file.close();

				cout << "\nCoding method: Arithmetic Method";
				cout << "\nCompression Data: \n";
				cout << " Symbol | Frequency | Segment" << endl;
				cout << "-----------------------------" << endl;

				for (int i = 0; i < count_str; i++)
				{
					if (i == 0)
						cout << " " << sym[i] << " | " << period[i] << " | " << "0" << " - " << period[i] << endl;
					else
						cout << " " << sym[i] << " | " << period[i] - period[i - 1] << " | " << period[i - 1] << " - " << period[i] << endl;
				}

				delete[] sym;
				sym = NULL;
				delete[] period;
				period = NULL;
			}
		}
		else
		{
			cout << "\nFile 'stats.txt' was corrupted!" << endl;
			exit(1);
		}
	}
};

class LZW : public BaseCompress
{
public:
	LZW(int Capacity = 256) : BaseCompress(Capacity) {}
	LZW(string s) : BaseCompress(s) {}
	~LZW() {}
	string compress()
	{
		int len_res = 0, diff, count = 0;
		string line, output;
		bool flag = true;
		int* res = new int[capacity];
		string* dict = new string[capacity];

		for (int i = 0; i < len; i++)
		{
			for (int j = 0; j < len_res; j++)
			{
				if (ptr[i] == dict[j][0])
				{
					flag = false;
					break;
				}
			}
			if (flag)
			{
				len_res++;
				dict[len_res - 1] = ptr[i];
			}
			flag = true;
		}
		diff = len_res;

		for (int i = 0; i < len; i++)
		{
			string temp;
			temp += ptr[i];
			if (i != len - 1)
				line = temp + ptr[i + 1];
			else
				line = temp;
			for (int j = 0; j < len_res; j++)
			{
				if (i != len - 1 && line == dict[j])
				{
					i++;
					if (i != len - 1)
						line += ptr[i + 1];
					j = 0;
				}
			}
			dict[len_res] = line;
			len_res++;
		}

		for (int i = diff; i < len_res; i++)
		{
			for (int j = 0; j < len_res; j++)
			{
				if (i != len_res - 1 && dict[j] == dict[i].substr(0, dict[i].length() - 1))
				{
					res[count] = j;
					count++;
					break;
				}
				else if (i == len_res - 1 && dict[j] == dict[i])
				{
					res[count] = j;
					break;
				}
			}
		}

		ofstream file("stats.txt");
		file << "3\n";
		for (int i = 0; i < len_res; i++)
			file << dict[i] << " ";
		file << '\n';
		for (int i = 0; i < len_res - diff; i++)
		{
			file << " " << res[i];
			output += to_string(res[i]);
		}
		file.close();

		delete[] res;
		res = NULL;
		delete[] dict;
		dict = NULL;
		delete[] dict;
		dict = NULL;
		return output;
	}
	string decompress()
	{
		string* dict = new string[capacity];
		int* num = new int[capacity];
		string output, line;
		int count_int = 0, count_str = 0;
		char c;
		bool flag = false;

		ifstream file("stats.txt");
		if (file.is_open())
		{
			file.get(c); file.get(c);
			while (!file.eof())
			{
				if (count_str < capacity - 1 && count_int < capacity - 1)
				{
					getline(file, line, ' ');
					if (line != "\n")
					{
						if (flag)
						{
							num[count_int] = stoi(line);
							count_int++;
						}
						else
						{
							dict[count_str] = line;
							count_str++;
						}
					}
					else
						flag = true;
				}
			}
			file.close();
		}
		else
		{
			cout << "File 'stats.txt' was corrupted!" << endl;
			exit(1);
		}

		for (int i = 0; i < count_int; i++)
			output += dict[num[i]];

		delete[] dict;
		dict = NULL;
		delete[] num;
		num = NULL;
		return output;
	}
	void showStats()
	{
		ifstream file("stats.txt");
		if (file.is_open())
		{
			char c;
			file.get(c);
			if (c == '3')
			{
				string line;
				getline(file, line);
				string* dict = new string[capacity];
				int* num = new int[capacity];
				line = "";
				int count_int = 0, count_str = 0;
				bool flag = false;

				while (!file.eof())
				{
					if (count_str < capacity - 1 && count_int < capacity - 1)
					{
						getline(file, line, ' ');
						if (line != "\n")
						{
							if (flag)
							{
								num[count_int] = stoi(line);
								count_int++;
							}
							else
							{
								dict[count_str] = line;
								count_str++;
							}
						}
						else
							flag = true;
					}
				}
				file.close();

				cout << "\nCoding method: Lempel-Ziv-Welch Algorithm";
				cout << "\nCompression Data: \n";
				cout << " Code | Enter | Current " << endl;
				cout << "------------------------" << endl;
				int diff = count_str - count_int;
				for (int i = diff; i < count_str; i++)
				{
					if (i != count_str - 1)
						cout << " " << num[i - diff] << " | " << dict[i] << " | " << dict[i].substr(0, dict[i].length() - 1) << endl;
					else
						cout << " " << num[i - diff] << " | " << dict[i] << " | " << dict[i] << endl;
				}

				delete[] dict;
				dict = NULL;
				delete[] num;
				num = NULL;
			}
		}
		else
		{
			cout << "\nFile 'stats.txt' was corrupted!" << endl;
			exit(1);
		}
	}
};

int main()
{
	string name;
	cout << "Write name of the input file: ";
	cin >> name;

	ifstream file(name);
	if (file.is_open())
	{
		string line1, line2;
		getline(file, line1);
		file.close();
		cout << "Your file includes: " << line1 << endl;
		string result_c, result_d;
		int key = 0;
		while (key != 1 && key != 2 && key != 3)
		{
			cout << "\nWhat method should your file be compressed with?" << endl << "1 - RLE / 2 - Arithmetic / 3 - LWZ" << endl;
			cin >> key;
			switch (key)
			{
			case 1:
			{
				RLE comp(line1);
				result_c = comp.compress();
				break;
			}
			case 2:
			{
				Arithmetic comp(line1);
				result_c = comp.compress();
				break;
			}
			case 3:
			{
				LZW comp(line1);
				result_c = comp.compress();
				break;
			}
			default:
				break;
			}

			if (key == 1 || key == 2 || key == 3)
			{
				cout << endl << "The result of compressing is: " << result_c << endl;
				ofstream file1("compressed.txt");
				file1 << result_c;
				file1.close();
				cout << "The result was successfully saved to 'compressed.txt'" << endl;
				int key1 = 0;
				while (key1 != 3)
				{
					bool back = false;
					cout << "\nWould you like to see compressing statistics or to decompress the result?" << endl << "1 - Statistics / 2 - Decompress / 3 - Back to compressing / 4 - Exit" << endl;
					cin >> key1;
					switch (key1)
					{
					case 1:
					{
						cout << "\nShowing statistics...";
						switch (key)
						{
						case 1:
						{
							RLE stat(line1);
							stat.showStats();
							break;
						}
						case 2:
						{
							Arithmetic stat(line1);
							stat.showStats();
							break;
						}
						case 3:
						{
							LZW stat(line1);
							stat.showStats();
							break;
						}
						default:
							break;
						}
						break;
					}
					case 2:
					{
						ifstream file("compressed.txt");
						if (file.is_open())
						{
							getline(file, line2);
							file.close();
							switch (key)
							{
							case 1:
							{
								RLE decomp(line2);
								result_d = decomp.decompress();
								break;
							}
							case 2:
							{
								Arithmetic decomp(line2);
								result_d = decomp.decompress();
								break;
							}
							case 3:
							{
								LZW decomp(line2);
								result_d = decomp.decompress();
								break;
							}
							default:
								cout << "There is no such variant, try again!" << endl;
								break;
							}
							cout << endl << "The result of decompressing is: " << result_d << endl;
							ofstream file1("decompressed.txt");
							file1 << result_d;
							file1.close();
							cout << "The result was successfully saved to 'decompressed.txt'" << endl;
						}
						else
						{
							cout << "File 'compressed.txt' was corrupted!" << endl;
							return 0;
						}
						break;
					}
					case 3:
						back = true;
						break;
					case 4:
						return 0;
					default:
						cout << "There is no such variant, try again!" << endl;
						break;
					}
					if (back)
					{
						key = 0;
						break;
					}
				}
			}
		}
	}
	else
		cout << endl << "There is no file with name '" << name << "'!" << endl;
}
