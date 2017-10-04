#include <Deracination/Straphanger/test/decortication/macros/common.cc>

void signal_acceptance_styler() {
	TFile* tf_in = TFile::Open("signal_acceptance_plots.root");
	TGraph* accept = (TGraph*) tf_in->Get("acceptance_sqto4j_sig");
	TCanvas* tc = new TCanvas("tc", "tc");
	
	gStyle->SetOptStat(0);
	accept->Draw("alp");
	accept->GetXaxis()->SetNdivisions(405);
	accept->GetXaxis()->SetRangeUser(0, 600);
	accept->GetXaxis()->SetTitle(get_xtitle("msq"));
	accept->GetYaxis()->SetTitle("Selection efficiency");
//	accept->GetYaxis()->SetTitleOffset(1.8);
//	TGaxis::SetMaxDigits(2);
//	cout << accept->GetMaximum() << endl;
	accept->SetMaximum(1e-2);
	accept->SetMinimum(5.0e-7);
	
	style_info(true, lum_string["all"], 0);
	style_cut("sig");
	
	tc->SetLogy();
	
	tc->SaveAs("eff_sqto4j_sig.pdf");
}
