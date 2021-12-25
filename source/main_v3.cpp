using namespace std;
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cmath>

#include "nbs_file_structure.h"
#include "shulker_output.h"
#include "nbs_v3.h"
#include "model.h"

//定义全局变量
int p_shulker[27][27][2], r_shulker[27][27][2]; //潜影盒内容[箱子号][格子号][物品，数量]
int error[100], num_of_err; //针对每音轨每8gt只能有一个音高进行纠错

//string model[92];


int main(int argc, char* argv[]) {
	load_model(model);

	for (int a = argc; a > 1; a--) {
		string path(argv[a - 1]);
		int pos = path.find_last_of("\\");
		string name(path, pos + 1, path.length() - pos - 5);
		string file_type(path, pos + name.length() + 2, path.length());
		if (file_type != "nbs") continue;

		ifstream infile(path, ios::in | ios::binary);
		nbs_file_v3 nbs(infile);
		infile.close();
		ofstream nbs_out(name + ".mcfunction", ios::out);
		ofstream nbs_out_txt(name + ".txt", ios::out);

		int inti_delay = 15;
		for (int i = 0; i < nbs.num_of_group; i++) {
			nbs_track_v3 p(nbs.track, nbs.group[i]);
			vector<output_track*> o;
			p.generate_outputs(o, inti_delay);
			for (int j = 0; j < o.size(); j++) {
				shulker<vector<nbs_note_v3*>> pitch(o[j]->notes, "pitch"), delay(o[j]->notes, "delay");
				generate_function(nbs_out, pitch.content, pitch.size, p.layer_name + " " + to_string(j + 1), "pitch");
				generate_function(nbs_out, delay.content, delay.size, p.layer_name + " " + to_string(j + 1), "delay");
				generate_txt<vector<nbs_note_v3*>>(nbs_out_txt, o[j]->notes, p.layer_name + " " + to_string(j + 1));
			}
		}
		nbs_out.close();
		nbs_out_txt.close();
	}
	
	return 0;
}

