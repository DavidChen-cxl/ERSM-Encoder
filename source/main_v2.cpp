using namespace std;
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>
#include "nbs_file_structure.h"
#include "nbs_v2.h"
#include "model.h"

int main(int argc, char* argv[]){
	load_model(model);

	for (int a = argc; a > 1; a--) {
		string path(argv[a - 1]);
		int pos = path.find_last_of("\\");
		string name(path, pos + 1, path.length() - pos - 5);
		string file_type(path, pos + name.length() + 2, path.length());
		if (file_type != "nbs") continue;

		ifstream infile(path, ios::in | ios::binary);
		nbs_file nbs(infile);
		infile.close();
		ofstream nbs_out(name + ".mcfunction", ios::out);
		ofstream nbs_out_txt(name + ".txt", ios::out);

		for (int i = 0; i < nbs.num_of_track; i++) {
			if (nbs.track[i].track_length >= 0) {
				nbs_track_v2 t(nbs.track[i]);
				if (t.err_report(nbs_out)) 
					continue;	
				else {
					shulker<vector<nbs_note_v2*>> pitch(t.notes, "pitch"), rhythm(t.notes, "rhythm");
					generate_function(nbs_out, pitch.content, pitch.size, t.layer_name, 1);
					generate_function(nbs_out, rhythm.content, rhythm.size, t.layer_name, 0);
					generate_txt<vector<nbs_note_v2*>>(nbs_out_txt, t.notes, t.layer_name);
				}
			}
		}

		nbs_out.close();
		nbs_out_txt.close();
	}
	return 0;
}