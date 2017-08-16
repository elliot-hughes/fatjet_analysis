
#include "/home/tote/decortication/macros/common.cc"

//vector<TH1*> make_parameter_plots(vector<TString> cuts, TFile* tf_out, int step=1) {
//	vector<TH1*> plots;
//	vector<int> ips = {4, 5, 6};
//	if (step == 2) ips = {2, 3};
//	for (int iip = 0; iip < ips.size(); ++ iip) {
//		int ip = ips[iip];
//		TString name = "step" + to_string(step) + "_p" + to_string(ip);
//		TH1* plot = new TH1F(name, "", cuts.size(), 0, cuts.size());
//		for (int i = 0; i < cuts.size(); ++ i) {
//			TString cut = cuts[i];
//			if (step == 1) cut = cut + "b";
//			TFile* tf_in = TFile::Open(TString("../analysis_step") + to_string(step) + "_fit_" + cut + ".root");
//			TH1* params = (TH1*) tf_in->Get(TString("step") + to_string(step) + "_params");
//			cout << cut << "  " << params->GetBinContent(ip) << "  " << params->GetBinError(ip) << endl;
//			plot->SetBinContent(i + 1, params->GetBinContent(ip));
//			plot->SetBinError(i + 1, params->GetBinError(ip));
//			tf_in->Close();
//		}
//		tf_out->WriteTObject(plot);
//		plots.push_back(plot);
//	}
//	return plots;
//}

void parameter_plotter() {
	vector<TString> cuts = {"sb", "sbl", "sbl42", "sbl43", "sbide"};
	vector<TString> labels = {"nominal", "loose", "loose-#tau_{42}", "loose-#tau_{43}", "inverted"};
	
	TFile* tf_out = new TFile("parameter_plots.root", "recreate");
	
	vector<TH1*> plots;
	vector<int> ips = {2, 3, 4, 5, 6};
	for (int iip = 0; iip < ips.size(); ++ iip) {
		int ip = ips[iip];
		for (int b = 0; b < 2; ++ b) {
			TString name = "b" + to_string(b) + "_p" + to_string(ip);
			TH1* plot = new TH1F(name, "", cuts.size(), 0, cuts.size());
			for (int i = 0; i < cuts.size(); ++ i) {
				TString cut = cuts[i];
				if (b == 1) cut = cut + "b";
				TFile* tf_in = TFile::Open(TString("../analysis_plots_fitsim_") + cut + "_f1.root");
				TH1* params = (TH1*) tf_in->Get("params");
				cout << cut << "  " << params->GetBinContent(ip) << "  " << params->GetBinError(ip) << endl;
				plot->SetBinContent(i + 1, params->GetBinContent(ip));
				plot->SetBinError(i + 1, params->GetBinError(ip));
				plot->GetXaxis()->SetBinLabel(i + 1, labels[i]);
				tf_in->Close();
			}
			tf_out->WriteTObject(plot);
			plots.push_back(plot);
		}
	}
}
