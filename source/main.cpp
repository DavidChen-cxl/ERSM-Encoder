using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "model.h"

//定义全局变量
string model[92];
int track[32][4096], pitch[32][1024], rhythm[32][1024], length[32]; //音轨时间序列（最多32音轨），后两者为每8gt
int p_shulker[27][27][2], r_shulker[27][27][2]; //潜影盒内容[箱子号][格子号][物品，数量]
int error[100], num_of_err; //针对每音轨每8gt只能有一个音高进行纠错

int debug_pointer_location = 0;
enum enum_read_type { Byte, Short, Int, String };

void type_read(ifstream& infile, enum_read_type type, int* buf) {
	char temp[5];
	if (type == Short) {
		infile.read(temp, 2);
		*buf = int((temp[1] << 8) + temp[0]);
		debug_pointer_location += 2;
	}
	else if (type == Int) {
		infile.read(temp, 4);
		*buf = int((temp[3] << 24) + (temp[2] << 16) + (temp[1] << 8) + temp[0]);
		debug_pointer_location += 4;
	}
	else if (type == Byte) {
		*buf = int(infile.get());
		debug_pointer_location += 1;
	}
}

int type_read(ifstream& infile, enum_read_type type) {
	char temp[5]; int buf;
	if (type == Short) {
		infile.read(temp, 2);
		return buf = int((temp[1] << 8) + temp[0]);
		debug_pointer_location += 2;
	}
	else if (type == Int) {
		infile.read(temp, 4);
		return buf = int((temp[3] << 24) + (temp[2] << 16) + (temp[1] << 8) + temp[0]);
		debug_pointer_location += 4;
	}
	else if (type == Byte) {
		return buf = int(infile.get());
		debug_pointer_location += 1;
	}
}

void type_read(ifstream& infile, enum_read_type type, char* buf) {
	if (type == String) {
		int str_len = type_read(infile, Int);
		infile.read(buf, str_len);
		debug_pointer_location += str_len;
	}
	else return;
}

void type_read(ifstream& infile, enum_read_type* types, int l, char* buf) {
	for (int i = 0; i < l; i++) {
		if (types[i] == String) type_read(infile, String, buf);
		else type_read(infile, types[i]);
	}
}

int construct_track_nbp(string* p, int* t) {	//从.nbp文件字符串生成时间序列音轨，返回长度
	int i = 0, pos = 0;
	while ((*p)[i + 2] != '|')
		i++;

	int loop_count = 0; int buf[3]; int j = 0, temp = 0, k;
	while ((*p)[i + 1] != '\0') {
		i++;
		if (loop_count < 8) {
			if ((*p)[i] == '|') {
				loop_count++;
				if (loop_count == 8) loop_count = 0;
				if (j) {
					for (k = 0; k < j; k++)
						temp += buf[k] * pow(10, j - k - 1);
					if (loop_count == 1) pos += temp;
					else t[pos] = temp - 33;
					j = 0;
					temp = 0;
				}
			}
			else if (loop_count == 0 || loop_count == 3) {
				buf[j] = int((*p)[i]) - 48;
				j++;
			}
		}
	}
	return pos / 4 + 1;
}

void construct_track_nbs(ifstream& infile, int t[][4096], int* l){
	int h_jump, v_jump, h_pos = -1, v_pos = -1, p, unused;
	bool tick_changed = false;
	char temp_peek;
	do {
		type_read(infile, Short, &h_jump);
		if (h_jump == 0)
			if (tick_changed) return;
		h_pos += h_jump;
		type_read(infile, Short, &v_jump);
		if (v_jump == 0) {
			v_pos = -1;
			tick_changed = true;
		}
		else {
			tick_changed = false;
			v_pos += v_jump;
			type_read(infile, Byte, &unused);
			p = (int)infile.get() - 33;
			t[v_pos][h_pos] = p;
			l[v_pos] = h_pos;
			type_read(infile, Short, &unused);
		}
	} while (true);

}

int generate_box(int s[][27][2], int* p, int l) {	//生成潜影盒内容（合并相同物品，分配格位），返回所需格数
	int size = 26, slot = 0, box = 0;
	s[0][0][0] = p[0];
	s[0][0][1] = 1;
	for (int i = 1; i < l; i++) {
		if (p[i] == p[i - 1]) {
			s[box][slot][1]++;
		}
		else {
			if (slot == size) {
				box++; slot = 0;
			}
			else slot++;
			s[box][slot][0] = p[i];
			s[box][slot][1] = 1;
		}
	}
	return box * 27 + slot;
}

void generate_function(ofstream& f, int s[][27][2], int c, string n, bool is_pitch) {	//根据每格物品生成指令
	int box = 0, slot = 0, size = 27;
	if(is_pitch) f << "give @p " << model[0] << n << " Pitch" << model[1];
	else f << "give @p " << model[0] << n << " Rhythm" << model[1];
	for (int i = 0; i <= c; i++) {
		box = i / size; slot = i % size;
		if (slot == 0) f << model[2];
		f << model[3] << model[9+52*(1-is_pitch) + s[box][slot][0]] << model[4] << " \\\"" << model[36+41*(1-is_pitch) + s[box][slot][0]] << model[5] << i % size << model[6] << s[box][slot][1] << "},";
		if (slot == 26) f << "]}},\"Slot\":" << box << model[6] << 1 << "},";
	}
	for (int i = c + 1; i < size * (c / size + 1); i++) {
		f << model[3] << model[7 + i % 2] << model[4] << " \\\"" << model[34 + i % 2] << model[5] << i % size << model[6] << 1 << "},";
	}
	if ((c + 1) % size != 0)
		f << "]}},\"Slot\":" << box << model[6] << 1 << "},]}}";
	else f << "]}}";
	f << "\n\n";
}

int check_8gt(int t) {
	int flag; int num_of_err = 0;
	for (int i = 0; i < length[t]; i++) {
		flag = 0;
		for (int j = 0; j < 4; j++) {
			if (track[t][4 * i + j] >= 0) {
				if (pitch[t][i] != track[t][4 * i + j]) {
					flag++;
					pitch[t][i] = track[t][4 * i + j];
				}
				rhythm[t][i] += pow(2, 3 - j);
			}
		}
		if (flag >= 2) error[num_of_err++] = i;
	}
	return num_of_err;
}

int find_track_begin(ifstream& infile) {
	int layers; char unused[257];
	enum_read_type nbs_head_1[4] = { Short,Byte,Byte,Short };
	type_read(infile, nbs_head_1, 4, unused);
	layers = type_read(infile, Short);
	enum_read_type nbs_head_2[17] = { String,String,String,String,Short,Byte,Byte,Byte,Int,Int,Int,Int,Int,String,Byte,Byte,Short };
	type_read(infile, nbs_head_2, 17, unused);
	return layers;
}

int main(int argc, char* argv[]) {

	load_model(model);
	
	bool nbp_write_flag = false; //nbp输出文件指示

	//文件主循环
	for (int a = argc; a > 1; a--) {
		//判断文件类型
		string path(argv[a - 1]);
		int pos = path.find_last_of("\\");
		string name(path, pos + 1, path.length() - pos - 5);
		string file_type(path, pos + name.length() + 2, path.length());
		if (file_type != "nbs" && file_type != "nbp") continue;

		//初始化音轨
		memset(track, -1, sizeof(track));
		memset(pitch, -1, sizeof(pitch));
		memset(rhythm, 0, sizeof(rhythm));
		memset(p_shulker, -1, sizeof(p_shulker));
		memset(r_shulker, 0, sizeof(r_shulker));

		//读入输入文件
		ifstream infile2(path, ios::in);

		//处理nbp，.nbp为带有文件头尾的简单ASCII文件，音轨部分与.nbs对应部分定义类似
		if (file_type == "nbp") {
			//创建输出文件
			if (!nbp_write_flag) {	
				ofstream nbp_out("nbp_output.mcfunction", ios::out);
				nbp_out.close();
				nbp_write_flag = true;
			}
			ofstream nbp_out("nbp_output.mcfunction", ios::out | ios::app);

			string nb; //从.nbp文件读取的音轨
			infile2 >> nb; 			
			length[0] = construct_track_nbp(&nb, track[0]);	//重构为时间序列

			num_of_err = check_8gt(0);
			if (num_of_err == 0) {
				//打包到潜影盒，获取所需格子数
				memset(p_shulker, -1, sizeof(p_shulker));
				memset(r_shulker, 0, sizeof(r_shulker));
				int p_scount = generate_box(p_shulker, pitch[0], length[0]);
				int	r_scount = generate_box(r_shulker, rhythm[0], length[0]);

				//生成指令
				generate_function(nbp_out, p_shulker, p_scount, name, 1);
				generate_function(nbp_out, r_shulker, r_scount, name, 0);
			}
			else {
				nbp_out << "#For Track " << name << " found errors at bar ";
				for (; num_of_err >= 1; num_of_err--)
					nbp_out << error[num_of_err - 1] + 1 << ", ";
				nbp_out << "\n\n";
			}
			nbp_out.close();
		}

		//处理nbs，其文件格式在opennbs.org/nbs有说明
		else {
			ofstream nbs_out(name + ".mcfunction", ios::out);
			int layer_count = find_track_begin(infile2);	//找到音轨起始位置
			memset(length, 0, sizeof(length));
			construct_track_nbs(infile2, track, length);

			for (int i = 0; i < min(32, layer_count) && length[i] != 0; i++) {
				int str_len; type_read(infile2, Int, &str_len);
				if (str_len == 0) {
					name = string("Layer ") + to_string(i + 1);
				}
				else {
					char temp[256];
					infile2.read(temp, str_len);
					name = string(temp, str_len);
				}
				if (type_read(infile2, Byte)) {type_read(infile2, Short); continue;}
				else type_read(infile2, Short);
				length[i] = length[i] / 4 + 1;
				num_of_err = check_8gt(i);
				if (num_of_err == 0) {
					//打包到潜影盒，获取所需格子数

					int p_scount = generate_box(p_shulker, pitch[i], length[i]);
					int	r_scount = generate_box(r_shulker, rhythm[i], length[i]);

					//生成指令
					generate_function(nbs_out, p_shulker, p_scount, name, 1);
					generate_function(nbs_out, r_shulker, r_scount, name, 0);
				}
				else {
					nbs_out << "#For Track " << name << " found errors at bar ";
					for (; num_of_err >= 1; num_of_err--)
						nbs_out << error[num_of_err - 1] + 1 << ", ";
					nbs_out << "\n\n";
				}
			}
			nbs_out.close();
		}
		infile2.close();
	}
	return 0;
}
