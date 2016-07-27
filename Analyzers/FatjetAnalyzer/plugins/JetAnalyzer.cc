/*#######################################################
# CMSSW EDAnalyzer                                      #
# Name: JetAnalyzer.cc                                  #
# Author: Elliot Hughes                                 #
#                                                       #
# Description: To analyze fatjets or B2G ntuples:       #
# select jets, create ntuples.                          #
#######################################################*/

// INCLUDES:
// System includes
#include <iostream>
#include <typeinfo>
#include <cmath>

/// User includes:
//// Basic includes:
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/FileBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"

//// Important class includes:
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/JetReco/interface/BasicJet.h"
#include "DataFormats/JetReco/interface/BasicJetCollection.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
///// JEC:
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"

//// Meta includes:
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//// Tools:
#include <boost/algorithm/string.hpp>

//// ROOT includes:
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TFile.h"
//#include "TFileDirectory.h"
#include "TTree.h"
#include "TBranch.h"
#include "TNtuple.h"
// \INCLUDES

// NAMESPACES:
using namespace std;
using namespace reco;
using namespace edm;
// \NAMESPACES

// STRUCTURES:
//struct sort_by_m {
//	bool operator() (PseudoJet jet1, PseudoJet jet2) {
//		return (jet1.m() > jet2.m());
//	}
//};
// \STRUCTURES

// CLASS DEFINITIONS:
class JetAnalyzer : public edm::EDAnalyzer {
	public:
		explicit JetAnalyzer(const edm::ParameterSet&);		// Set the class argument to be (a reference to) a parameter set (?)
		~JetAnalyzer();		// Create the destructor.
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

	private:
		virtual void beginJob();
		virtual void process_jets_pf(const edm::Event&, string, EDGetTokenT<vector<pat::Jet>>);
		virtual void process_jets_gn(const edm::Event&, string, EDGetTokenT<vector<reco::GenJet>>);
		virtual void process_jets_maod(const edm::Event&, string, EDGetTokenT<vector<pat::Jet>>);
		virtual void process_electrons_pf(const edm::Event&, EDGetTokenT<vector<pat::Electron>>);
		virtual void process_muons_pf(const edm::Event&, EDGetTokenT<vector<pat::Muon>>);
		virtual void process_tauons_pf(const edm::Event&, EDGetTokenT<vector<pat::Tau>>);
		virtual void process_photons_pf(const edm::Event&, EDGetTokenT<vector<pat::Photon>>);
		virtual void analyze(const edm::Event&, const edm::EventSetup&);
		virtual void endJob();
		virtual void beginRun(const edm::Run&, const edm::EventSetup&);
		virtual void endRun(const edm::Run&, const edm::EventSetup&);
		virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&);
		virtual void endLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&);
		virtual void respondToOpenInputFile(const edm::FileBlock&);

	// Member data
	/// Configuration variables (filled by setting the python configuration file)
	bool v_;                    // Verbose control
	bool is_data_;
	int in_type_;               // Input type (0: B2G, 1: fatjets)
	double sigma_, weight_, cut_pt_;
	bool make_gen_, make_pf_;		// Controls to make gen fatjets or pf fatjets
	string jec_version_;
	/// Parameters
	double L;
	// Basic fatjet variables
	// Algorithm variables
	int n_event, n_event_sel, n_sel_lead, counter, n_error_g, n_error_q, n_error_sq, n_error_sq_match, n_error_m, n_error_sort;
	
	// Input collections:
	vector<string> jet_names, jet_types;
	vector<string> lep_names, lep_types;
	
	// JEC info:
	string jec_prefix;
	vector<string> jec_ak4_files, jmc_ak4_files, jec_ak8_files, jmc_ak8_files;
	FactorizedJetCorrector *jec_corrector_ak4, *jmc_corrector_ak4, *jec_corrector_ak8, *jmc_corrector_ak8;
	
	// Ntuple information:
	vector<string> jet_variables, lep_variables, event_variables;
	map<string, map<string, vector<double>>> branches;
	map<string, TTree*> ttrees;
	map<string, map<string, TBranch*>> tbranches;
	TTree* tt;
	
	// Event variables:
	double pt_hat;
	double rho;
	double npv;
	
	// Tokens:
	EDGetTokenT<GenEventInfoProduct> genInfo_;
	EDGetTokenT<double> rhoInfo_;
	EDGetTokenT<reco::VertexCollection> vertexCollection_;
	EDGetTokenT<vector<pat::Jet>> ak4PFCollection_;
	EDGetTokenT<vector<pat::Jet>> ak8PFCollection_;
	EDGetTokenT<vector<pat::Jet>> ca12PFCollection_;
	EDGetTokenT<vector<reco::GenJet>> ak4GNCollection_;
	EDGetTokenT<vector<reco::GenJet>> ak8GNCollection_;
	EDGetTokenT<vector<reco::GenJet>> ca12GNCollection_;
	EDGetTokenT<vector<pat::Jet>> ak4MAODCollection_;
	EDGetTokenT<vector<pat::Jet>> ak8MAODCollection_;
	EDGetTokenT<vector<pat::Electron>> electronCollection_;
	EDGetTokenT<vector<pat::Muon>> muonCollection_;
	EDGetTokenT<vector<pat::Tau>> tauCollection_;
	EDGetTokenT<vector<pat::Photon>> photonCollection_;
};
// \CLASS DEFINITIONS

// Constants, enums and typedefs
	// Simple algorithm variables:
	/// Custom iterators:
	typedef map<string, vector<double>>::iterator branch_it_inner;
	typedef map<string, map<string, vector<double>>>::iterator branch_it_outer;
	
	
	// DEFINE CUTS
//	float cut_dm = 25;

// static data member definitions

// constructors and destructors
JetAnalyzer::JetAnalyzer(const edm::ParameterSet& iConfig) :
	v_(iConfig.getParameter<bool>("v")),
	is_data_(iConfig.getParameter<bool>("is_data")),
	in_type_(iConfig.getParameter<int>("in_type")),
	sigma_(iConfig.getParameter<double>("sigma")),
	weight_(iConfig.getParameter<double>("weight")),
	cut_pt_(iConfig.getParameter<double>("cut_pt")),
	jec_version_(iConfig.getParameter<string>("jec_version")),
	// Consume statements:
	genInfo_(consumes<GenEventInfoProduct>(iConfig.getParameter<InputTag>("genInfo"))),
	rhoInfo_(consumes<double>(iConfig.getParameter<InputTag>("rhoInfo"))),
	vertexCollection_(consumes<reco::VertexCollection>(iConfig.getParameter<InputTag>("vertexCollection"))),
	ak4PFCollection_(consumes<vector<pat::Jet>>(iConfig.getParameter<InputTag>("ak4PFCollection"))),
	ak8PFCollection_(consumes<vector<pat::Jet>>(iConfig.getParameter<InputTag>("ak8PFCollection"))),
	ca12PFCollection_(consumes<vector<pat::Jet>>(iConfig.getParameter<InputTag>("ca12PFCollection"))),
	ak4GNCollection_(consumes<vector<reco::GenJet>>(iConfig.getParameter<InputTag>("ak4GNCollection"))),
	ak8GNCollection_(consumes<vector<reco::GenJet>>(iConfig.getParameter<InputTag>("ak8GNCollection"))),
	ca12GNCollection_(consumes<vector<reco::GenJet>>(iConfig.getParameter<InputTag>("ca12GNCollection"))),
	ak4MAODCollection_(consumes<vector<pat::Jet>>(iConfig.getParameter<InputTag>("ak4MAODCollection"))),
	ak8MAODCollection_(consumes<vector<pat::Jet>>(iConfig.getParameter<InputTag>("ak8MAODCollection"))),
	electronCollection_(consumes<vector<pat::Electron>>(iConfig.getParameter<InputTag>("electronCollection"))),
	muonCollection_(consumes<vector<pat::Muon>>(iConfig.getParameter<InputTag>("muonCollection"))),
	tauCollection_(consumes<vector<pat::Tau>>(iConfig.getParameter<InputTag>("tauCollection"))),
	photonCollection_(consumes<vector<pat::Photon>>(iConfig.getParameter<InputTag>("photonCollection")))
{
//do what ever initialization is needed
	// Parameters:
	L = 10000;                // Luminosity (in inverse pb)
	
	// Collections setup:
	/// "jet"
	jet_names = {"ak4", "ak8", "ca12"};
	jet_types = {"pf", "gn", "maod"};
	jet_variables = {		// List of event branch variables for each collection.
		"phi", "eta", "y", "px", "py", "pz", "e", "pt",
		"M",          // Ungroomed mass
		"m_t",        // Trimmed mass
		"m_p",        // Pruned mass
		"m_s",        // Soft Drop mass
		"m_f",        // Filtered mass
		"tau1",       // Nsubjettiness 1
		"tau2",       // Nsubjettiness 2
		"tau3",       // Nsubjettiness 3
		"tau4",       // Nsubjettiness 4
		"tau5",       // Nsubjettiness 5
		"ht",         // Sum of jet pTs. In the case of AK8, it's the sum of the jet pTs with pT > 150 GeV
		"bd_te",
		"bd_tp",
		"bd_csv",
		"bd_cisv",
		"jec",
		"jmc",
	};
	
	/// "lep"
	lep_names = {"le", "lm", "lt", "lp"};
	lep_types = {"pf"};
	lep_variables = {		// List of event branch variables for each collection.
		"phi", "eta", "y", "px", "py", "pz", "e", "pt", "m",
	};
	
	/// "event"
	event_variables = {
		"pt_hat",
		"sigma",      // Cross section of the event
		"nevent",     // The unique event number
//		"nevents",    // The total number of events that were run over
		"w",          // Event weight
		"rho",
		"npv",        // Number of primary vertices
	};
	
	// Ntuple setup:
	edm::Service<TFileService> fs;		// Open output services
	
	/// Event-by-event variables:
	ttrees["events"] = fs->make<TTree>();
	ttrees["events"]->SetName("events");
	
	//// "jet":
	for (vector<string>::const_iterator i = jet_names.begin(); i != jet_names.end(); i++) {
		string name = *i;
		for (vector<string>::const_iterator j = jet_types.begin(); j != jet_types.end(); j++) {
			string type = *j;
			string name_type = name + "_" + type;
			for (vector<string>::const_iterator k = jet_variables.begin(); k != jet_variables.end(); k++) {
				string variable = *k;
				string branch_name = name + "_" + type + "_" + variable;
				tbranches[name_type][variable] = ttrees["events"]->Branch(branch_name.c_str(), &(branches[name_type][variable]), 64000, 0);
			}
		}
	}
	//// "lep":
	for (vector<string>::const_iterator i = lep_names.begin(); i != lep_names.end(); i++) {
		string name = *i;
		for (vector<string>::const_iterator j = lep_types.begin(); j != lep_types.end(); j++) {
			string type = *j;
			string name_type = name + "_" + type;
			for (vector<string>::const_iterator k = lep_variables.begin(); k != lep_variables.end(); k++) {
				string variable = *k;
				string branch_name = name + "_" + type + "_" + variable;
				tbranches[name_type][variable] = ttrees["events"]->Branch(branch_name.c_str(), &(branches[name_type][variable]), 64000, 0);
			}
		}
	}
	//// "event":
	for (vector<string>::iterator i = event_variables.begin(); i != event_variables.end(); i++) {
		string branch_name = *i;
		tbranches["event"][*i] = ttrees["events"]->Branch(branch_name.c_str(), &(branches["event"][*i]), 64000, 0);
	}
	
	// JEC setup:
	if (is_data_) jec_prefix = jec_version_ + "_DATA";
	else jec_prefix = jec_version_ + "_MC";
	
	/// AK4 JEC setup:
	jec_ak4_files.push_back(jec_prefix + "_L1FastJet_AK4PFchs.txt");
	jec_ak4_files.push_back(jec_prefix + "_L2Relative_AK4PFchs.txt");
	jec_ak4_files.push_back(jec_prefix + "_L3Absolute_AK4PFchs.txt");
	if (is_data_) jec_ak4_files.push_back(jec_prefix + "_L2L3Residual_AK4PFchs.txt");
	vector<JetCorrectorParameters> jec_parameters_ak4;
	for (vector<string>::const_iterator jec_file = jec_ak4_files.begin(); jec_file != jec_ak4_files.end(); ++jec_file) {
//		cout << *jec_file << endl;
		jec_parameters_ak4.push_back(*(new JetCorrectorParameters(*jec_file)));
	}
	jec_corrector_ak4 = new FactorizedJetCorrector(jec_parameters_ak4);
	
	/// AK4 JMC setup:
	jmc_ak4_files.push_back(jec_prefix + "_L2Relative_AK4PFchs.txt");
	jmc_ak4_files.push_back(jec_prefix + "_L3Absolute_AK4PFchs.txt");
	if (is_data_) jmc_ak4_files.push_back(jec_prefix + "_L2L3Residual_AK4PFchs.txt");
	vector<JetCorrectorParameters> jmc_parameters_ak4;
	for (vector<string>::const_iterator jmc_file = jmc_ak4_files.begin(); jmc_file != jmc_ak4_files.end(); ++jmc_file) {
//		cout << *jmc_file << endl;
		jmc_parameters_ak4.push_back(*(new JetCorrectorParameters(*jmc_file)));
	}
	jmc_corrector_ak4 = new FactorizedJetCorrector(jmc_parameters_ak4);
//	
	/// AK8 JEC setup:
	jec_ak8_files.push_back(jec_prefix + "_L1FastJet_AK8PFchs.txt");
	jec_ak8_files.push_back(jec_prefix + "_L2Relative_AK8PFchs.txt");
	jec_ak8_files.push_back(jec_prefix + "_L3Absolute_AK8PFchs.txt");
	if (is_data_) jec_ak8_files.push_back(jec_prefix + "_L2L3Residual_AK8PFchs.txt");
	vector<JetCorrectorParameters> jec_parameters_ak8;
	for (vector<string>::const_iterator jec_file = jec_ak8_files.begin(); jec_file != jec_ak8_files.end(); ++jec_file) {
//		cout << *jec_file << endl;
		jec_parameters_ak8.push_back(*(new JetCorrectorParameters(*jec_file)));
	}
	jec_corrector_ak8 = new FactorizedJetCorrector(jec_parameters_ak8);
	
	/// AK8 JMC setup:
	jmc_ak8_files.push_back(jec_prefix + "_L2Relative_AK8PFchs.txt");
	jmc_ak8_files.push_back(jec_prefix + "_L3Absolute_AK8PFchs.txt");
	if (is_data_) jmc_ak8_files.push_back(jec_prefix + "_L2L3Residual_AK8PFchs.txt");
	vector<JetCorrectorParameters> jmc_parameters_ak8;
	for (vector<string>::const_iterator jmc_file = jmc_ak8_files.begin(); jmc_file != jmc_ak8_files.end(); ++jmc_file) {
//		cout << *jmc_file << endl;
		jmc_parameters_ak8.push_back(*(new JetCorrectorParameters(*jmc_file)));
	}
	jmc_corrector_ak8 = new FactorizedJetCorrector(jmc_parameters_ak8);
	
	
	// Debug:
	cout << endl;
	cout << "Starting the Jet Analyzer..." << endl;
	cout << "in_type = " << in_type_ << endl;
	cout << "v = " << v_ << "." << endl;
	cout << "jec_prefix = " << jec_prefix << endl;
}

// DEFINE THE DESTRUCTOR
JetAnalyzer::~JetAnalyzer()
{
	cout << "======================================================" << endl;
	cout << "Error records:" << endl;
	cout << "This hasn't been implemented, yet." << endl;
//	cout << "There were " << n_event << " events." << endl;
//	cout << "There were " << n_event_sel << " selected events." << endl;
//	cout << "There were " << n_sel_lead << " (" << n_sel_lead / (float)n_event_sel * 100 << "%) events in which the first two jets were selected." << endl;
//	cout << "* Squark number errors: " << n_error_sq << endl;
//	cout << "* Quark number errors: " << n_error_q << endl;
//	cout << "* Squark matching errors: " << n_error_sq_match << endl;
	cout << "======================================================" << endl;
}

// ------------ called once each job just before starting event loop  ------------
void JetAnalyzer::beginJob()
{
	n_event = 0;
//	cout << "maxEvents = " << nevents_ << endl;
//	cout << "Running over " << nevents_ << " events ..." << endl;
}

// CLASS METHODS ("method" = "member function")

/// PF jets method:
void JetAnalyzer::process_jets_pf(const edm::Event& iEvent, string algo, EDGetTokenT<vector<pat::Jet>> token) {
	// Arguments:
	string type = "pf";
	string algo_type = algo + "_" + type;
	
	Handle<vector<pat::Jet>> jets;
	iEvent.getByToken(token, jets);

	// Print some info:
//	if (v_) {cout << ">> There are " << jets->size() << " jets in the " << algo_type << " collection." << endl;}
	
	// Loop over the collection:
	double ht = 0;
	for (vector<pat::Jet>::const_iterator jet = jets->begin(); jet != jets->end(); ++ jet) {
		// Define some useful event variables:
		double M = jet->mass();
		double m_t = jet->userFloat(algo + string("PFJetsCHSTrimmedMass"));
		double m_p = jet->userFloat(algo + string("PFJetsCHSPrunedMass"));
		double m_s = jet->userFloat(algo + string("PFJetsCHSSoftDropMass"));
		double m_f = jet->userFloat(algo + string("PFJetsCHSFilteredMass"));
		double tau1 = -1;
		double tau2 = -1;
		double tau3 = -1;
		double tau4 = -1;
		double tau5 = -1;
		if (algo != "ak4"){
			tau1 = jet->userFloat(string("Njettiness") + boost::to_upper_copy<string>(algo) + string("CHS:tau1"));
			tau2 = jet->userFloat(string("Njettiness") + boost::to_upper_copy<string>(algo) + string("CHS:tau2"));
			tau3 = jet->userFloat(string("Njettiness") + boost::to_upper_copy<string>(algo) + string("CHS:tau3"));
			tau4 = jet->userFloat(string("Njettiness") + boost::to_upper_copy<string>(algo) + string("CHS:tau4"));
			tau5 = jet->userFloat(string("Njettiness") + boost::to_upper_copy<string>(algo) + string("CHS:tau5"));
		}
		double px = jet->px();
		double py = jet->py();
		double pz = jet->pz();
		double e = jet->energy();
		double pt = jet->pt();
		double phi = jet->phi();
		double eta = jet->eta();
		double y = jet->y();
		double A = jet->jetArea();
		if (algo == "ak8") {
			if (pt > 150) {
				ht += pt;
			}
		}
		else {
			ht += pt;
		}
		
		// Get JECs:
		double jec = -1;
		double jmc = -1;
		if (algo == "ak4") {
			jec_corrector_ak4->setJetPt(pt);
			jec_corrector_ak4->setJetEta(eta);
			jec_corrector_ak4->setJetPhi(phi);
			jec_corrector_ak4->setJetE(e);
			jec_corrector_ak4->setJetA(A);
			jec_corrector_ak4->setRho(rho);
			jec_corrector_ak4->setNPV(npv);
			jec = jec_corrector_ak4->getCorrection();
			
			jmc_corrector_ak4->setJetPt(pt);
			jmc_corrector_ak4->setJetEta(eta);
			jmc_corrector_ak4->setJetPhi(phi);
			jmc_corrector_ak4->setJetE(e);
			jmc_corrector_ak4->setJetA(A);
			jmc_corrector_ak4->setRho(rho);
			jmc_corrector_ak4->setNPV(npv);
			jmc = jmc_corrector_ak4->getCorrection();
		}
		else {
			jec_corrector_ak8->setJetPt(pt);
			jec_corrector_ak8->setJetEta(eta);
			jec_corrector_ak8->setJetPhi(phi);
			jec_corrector_ak8->setJetE(e);
			jec_corrector_ak8->setJetA(A);
			jec_corrector_ak8->setRho(rho);
			jec_corrector_ak8->setNPV(npv);
			jec = jec_corrector_ak8->getCorrection();
			
			jmc_corrector_ak8->setJetPt(pt);
			jmc_corrector_ak8->setJetEta(eta);
			jmc_corrector_ak8->setJetPhi(phi);
			jmc_corrector_ak8->setJetE(e);
			jmc_corrector_ak8->setJetA(A);
			jmc_corrector_ak8->setRho(rho);
			jmc_corrector_ak8->setNPV(npv);
			jmc = jmc_corrector_ak8->getCorrection();
		}
		
		// Fill branches:
		if (pt > cut_pt_) {
			branches[algo_type]["phi"].push_back(phi);
			branches[algo_type]["eta"].push_back(eta);
			branches[algo_type]["y"].push_back(y);
			branches[algo_type]["px"].push_back(px);
			branches[algo_type]["py"].push_back(py);
			branches[algo_type]["pz"].push_back(pz);
			branches[algo_type]["e"].push_back(e);
			branches[algo_type]["pt"].push_back(pt);
			branches[algo_type]["M"].push_back(M);
			branches[algo_type]["m_t"].push_back(m_t);
			branches[algo_type]["m_p"].push_back(m_p);
			branches[algo_type]["m_s"].push_back(m_s);
			branches[algo_type]["m_f"].push_back(m_f);
			branches[algo_type]["tau1"].push_back(tau1);
			branches[algo_type]["tau2"].push_back(tau2);
			branches[algo_type]["tau3"].push_back(tau3);
			branches[algo_type]["tau4"].push_back(tau4);
			branches[algo_type]["tau5"].push_back(tau5);
			branches[algo_type]["jec"].push_back(jec);
			branches[algo_type]["jmc"].push_back(jmc);
		}
	}		// :End collection loop
	
	branches[algo_type]["ht"].push_back(ht);
}

/// GN jets method:
void JetAnalyzer::process_jets_gn(const edm::Event& iEvent, string algo, EDGetTokenT<vector<reco::GenJet>> token) {
	// Arguments:
	string type = "gn";
	string algo_type = algo + "_" + type;
	
	Handle<vector<reco::GenJet>> jets;
	iEvent.getByToken(token, jets);

	// Print some info:
//	if (v_) {cout << ">> There are " << jets->size() << " jets in the " << algo_type << " collection." << endl;}
	
	// Loop over the collection:
	double ht = 0;
	for (vector<reco::GenJet>::const_iterator jet = jets->begin(); jet != jets->end(); ++ jet) {
		// Define some useful event variables:
		double M = jet->mass();
		double m_t = -1;
		double m_p = -1;
		double m_s = -1;
		double m_f = -1;
		double tau1 = -1;
		double tau2 = -1;
		double tau3 = -1;
		double tau4 = -1;
		double tau5 = -1;
		double px = jet->px();
		double py = jet->py();
		double pz = jet->pz();
		double e = jet->energy();
		double pt = jet->pt();
		double phi = jet->phi();
		double eta = jet->eta();
		double y = jet->y();
		if (algo == "ak8") {
			if (pt > 150) {
				ht += pt;
			}
		}
		else {
			ht += pt;
		}
		
		// Fill branches:
		if (pt > cut_pt_) {
			branches[algo_type]["phi"].push_back(phi);
			branches[algo_type]["eta"].push_back(eta);
			branches[algo_type]["y"].push_back(y);
			branches[algo_type]["px"].push_back(px);
			branches[algo_type]["py"].push_back(py);
			branches[algo_type]["pz"].push_back(pz);
			branches[algo_type]["e"].push_back(e);
			branches[algo_type]["pt"].push_back(pt);
			branches[algo_type]["M"].push_back(M);
			branches[algo_type]["m_t"].push_back(m_t);
			branches[algo_type]["m_p"].push_back(m_p);
			branches[algo_type]["m_s"].push_back(m_s);
			branches[algo_type]["m_f"].push_back(m_f);
			branches[algo_type]["tau1"].push_back(tau1);
			branches[algo_type]["tau2"].push_back(tau2);
			branches[algo_type]["tau3"].push_back(tau3);
			branches[algo_type]["tau4"].push_back(tau4);
			branches[algo_type]["tau5"].push_back(tau5);
//			branches[algo_type]["jec"].push_back(-1);
//			branches[algo_type]["jmc"].push_back(-1);
		}
	}		// :End collection loop
	
	branches[algo_type]["ht"].push_back(ht);
}

/// MAOD jets method:
void JetAnalyzer::process_jets_maod(const edm::Event& iEvent, string algo, EDGetTokenT<vector<pat::Jet>> token) {
	// Arguments:
	string type = "maod";
	string algo_type = algo + "_" + type;
	
	Handle<vector<pat::Jet>> jets;
	iEvent.getByToken(token, jets);

	// Print some info:
//	if (v_) {cout << ">> There are " << jets->size() << " jets in the " << algo_type << " collection." << endl;}
	
	// Loop over the collection:
	double ht = 0;
	for (vector<pat::Jet>::const_iterator jet = jets->begin(); jet != jets->end(); ++ jet) {
		// Define some useful event variables:
		double M = jet->mass();
		double m_t = -1;
		double m_p = -1;
		double m_s = -1;
		double m_f = -1;
		double tau1 = -1;
		double tau2 = -1;
		double tau3 = -1;
		double tau4 = -1;
		double tau5 = -1;
		double px = jet->px();
		double py = jet->py();
		double pz = jet->pz();
		double e = jet->energy();
		double pt = jet->pt();
		double phi = jet->phi();
		double eta = jet->eta();
		double y = jet->y();
		if (algo == "ak8") {
			if (pt > 150) {
				ht += pt;
			}
		}
		else {
			ht += pt;
		}
		
		// Fill branches:
		if (pt > cut_pt_) {
			branches[algo_type]["phi"].push_back(phi);
			branches[algo_type]["eta"].push_back(eta);
			branches[algo_type]["y"].push_back(y);
			branches[algo_type]["px"].push_back(px);
			branches[algo_type]["py"].push_back(py);
			branches[algo_type]["pz"].push_back(pz);
			branches[algo_type]["e"].push_back(e);
			branches[algo_type]["pt"].push_back(pt);
			branches[algo_type]["M"].push_back(M);
			branches[algo_type]["m_t"].push_back(m_t);
			branches[algo_type]["m_p"].push_back(m_p);
			branches[algo_type]["m_s"].push_back(m_s);
			branches[algo_type]["m_f"].push_back(m_f);
			branches[algo_type]["tau1"].push_back(tau1);
			branches[algo_type]["tau2"].push_back(tau2);
			branches[algo_type]["tau3"].push_back(tau3);
			branches[algo_type]["tau4"].push_back(tau4);
			branches[algo_type]["tau5"].push_back(tau5);
			branches[algo_type]["bd_te"].push_back(jet->bDiscriminator("pfTrackCountingHighEffBJetTags"));
			branches[algo_type]["bd_tp"].push_back(jet->bDiscriminator("pfTtrackCountingHighPurBJetTags"));
			branches[algo_type]["bd_csv"].push_back(jet->bDiscriminator("pfCombinedSecondaryVertexV2BJetTags"));
			branches[algo_type]["bd_cisv"].push_back(jet->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags"));
//			branches[algo_type]["jec"].push_back(-1);
//			branches[algo_type]["jmc"].push_back(-1);
		}
	}		// :End collection loop
	
	branches[algo_type]["ht"].push_back(ht);
}

/// Electrons method:
void JetAnalyzer::process_electrons_pf(const edm::Event& iEvent, EDGetTokenT<vector<pat::Electron>> token) {
	// Arguments:
	string name = "le";
	string type = "pf";
	string name_type = name + "_" + type;
	
	Handle<vector<pat::Electron>> leps;
	iEvent.getByToken(token, leps);
	
	// Loop over the collection:
	for (vector<pat::Electron>::const_iterator lep = leps->begin(); lep != leps->end(); ++ lep) {
		// Define some useful event variables:
		double m = lep->mass();
		double px = lep->px();
		double py = lep->py();
		double pz = lep->pz();
		double e = lep->energy();
		double pt = lep->pt();
		double phi = lep->phi();
		double eta = lep->eta();
		double y = lep->y();
		
		// Fill branches:
		if (pt > 5) {
			branches[name_type]["phi"].push_back(phi);
			branches[name_type]["eta"].push_back(eta);
			branches[name_type]["y"].push_back(y);
			branches[name_type]["px"].push_back(px);
			branches[name_type]["py"].push_back(py);
			branches[name_type]["pz"].push_back(pz);
			branches[name_type]["e"].push_back(e);
			branches[name_type]["pt"].push_back(pt);
			branches[name_type]["m"].push_back(m);
		}
	}		// :End collection loop
}

/// Muons method:
void JetAnalyzer::process_muons_pf(const edm::Event& iEvent, EDGetTokenT<vector<pat::Muon>> token) {
	// Arguments:
	string name = "lm";
	string type = "pf";
	string name_type = name + "_" + type;
	
	Handle<vector<pat::Muon>> leps;
	iEvent.getByToken(token, leps);
	
	// Loop over the collection:
	for (vector<pat::Muon>::const_iterator lep = leps->begin(); lep != leps->end(); ++ lep) {
		// Define some useful event variables:
		double m = lep->mass();
		double px = lep->px();
		double py = lep->py();
		double pz = lep->pz();
		double e = lep->energy();
		double pt = lep->pt();
		double phi = lep->phi();
		double eta = lep->eta();
		double y = lep->y();
		
		// Fill branches:
		if (pt > 5) {
			branches[name_type]["phi"].push_back(phi);
			branches[name_type]["eta"].push_back(eta);
			branches[name_type]["y"].push_back(y);
			branches[name_type]["px"].push_back(px);
			branches[name_type]["py"].push_back(py);
			branches[name_type]["pz"].push_back(pz);
			branches[name_type]["e"].push_back(e);
			branches[name_type]["pt"].push_back(pt);
			branches[name_type]["m"].push_back(m);
		}
	}		// :End collection loop
}

/// Tauons method:
void JetAnalyzer::process_tauons_pf(const edm::Event& iEvent, EDGetTokenT<vector<pat::Tau>> token) {
	// Arguments:
	string name = "lt";
	string type = "pf";
	string name_type = name + "_" + type;
	
	Handle<vector<pat::Tau>> leps;
	iEvent.getByToken(token, leps);
	
	// Loop over the collection:
	for (vector<pat::Tau>::const_iterator lep = leps->begin(); lep != leps->end(); ++ lep) {
		// Define some useful event variables:
		double m = lep->mass();
		double px = lep->px();
		double py = lep->py();
		double pz = lep->pz();
		double e = lep->energy();
		double pt = lep->pt();
		double phi = lep->phi();
		double eta = lep->eta();
		double y = lep->y();
		
		// Fill branches:
		if (pt > 5) {
			branches[name_type]["phi"].push_back(phi);
			branches[name_type]["eta"].push_back(eta);
			branches[name_type]["y"].push_back(y);
			branches[name_type]["px"].push_back(px);
			branches[name_type]["py"].push_back(py);
			branches[name_type]["pz"].push_back(pz);
			branches[name_type]["e"].push_back(e);
			branches[name_type]["pt"].push_back(pt);
			branches[name_type]["m"].push_back(m);
		}
	}		// :End collection loop
}

/// Photons method:
void JetAnalyzer::process_photons_pf(const edm::Event& iEvent, EDGetTokenT<vector<pat::Photon>> token) {
	// Arguments:
	string name = "lp";
	string type = "pf";
	string name_type = name + "_" + type;
	
	Handle<vector<pat::Photon>> leps;
	iEvent.getByToken(token, leps);
	
	// Loop over the collection:
	for (vector<pat::Photon>::const_iterator lep = leps->begin(); lep != leps->end(); ++ lep) {
		// Define some useful event variables:
		double m = lep->mass();
		double px = lep->px();
		double py = lep->py();
		double pz = lep->pz();
		double e = lep->energy();
		double pt = lep->pt();
		double phi = lep->phi();
		double eta = lep->eta();
		double y = lep->y();
		
		// Fill branches:
		if (pt > 5) {
			branches[name_type]["phi"].push_back(phi);
			branches[name_type]["eta"].push_back(eta);
			branches[name_type]["y"].push_back(y);
			branches[name_type]["px"].push_back(px);
			branches[name_type]["py"].push_back(py);
			branches[name_type]["pz"].push_back(pz);
			branches[name_type]["e"].push_back(e);
			branches[name_type]["pt"].push_back(pt);
			branches[name_type]["m"].push_back(m);
		}
	}		// :End collection loop
}
// ------------ called for each event  ------------
void JetAnalyzer::analyze(
	const edm::Event& iEvent,
	const edm::EventSetup& iSetup
){
//	cout << "HERE 324" << endl;
	n_event ++;		// Increment the event counter by one. For the first event, n_event = 1.
	
	// Get objects from event:
	if (in_type_ == 0) {
		if (n_event == 1) {
			cout << "You wanted to run over a B2G ntuple. This isn't implemented, yet ..." << endl;
		}
	}
	else if (in_type_ == 1) {
		if (v_) {cout << "Running over JetToolbox collections ..." << endl;}
		
		// Clear branches:
		/// "jet":
		for (vector<string>::const_iterator i = jet_names.begin(); i != jet_names.end(); i++) {
			string name = *i;
			for (vector<string>::const_iterator j = jet_types.begin(); j != jet_types.end(); j++) {
				string type = *j;
				string name_type = name + "_" + type;
				for (vector<string>::iterator k = jet_variables.begin(); k != jet_variables.end(); k++) {
					branches[name_type][*k].clear();
				}
			}
		}
		/// "lep":
		for (vector<string>::const_iterator i = lep_names.begin(); i != lep_names.end(); i++) {
			string name = *i;
			for (vector<string>::const_iterator j = lep_types.begin(); j != lep_types.end(); j++) {
				string type = *j;
				string name_type = name + "_" + type;
				for (vector<string>::iterator k = lep_variables.begin(); k != lep_variables.end(); k++) {
					branches[name_type][*k].clear();
				}
			}
		}
		/// "event":
		for (vector<string>::iterator i = event_variables.begin(); i != event_variables.end(); i++) {
			branches["event"][*i].clear();
		}
		
		// Get event-wide variables:
		/// pT-hat:
		pt_hat = -1;
		edm::Handle<GenEventInfoProduct> gn_event_info;
		iEvent.getByToken(genInfo_, gn_event_info);
		if (gn_event_info->hasBinningValues()) {
			pt_hat = gn_event_info->binningValues()[0];
		}
		/// Rho:
		rho = -1;
		edm::Handle<double> rho_;
		iEvent.getByToken(rhoInfo_, rho_);
		rho = *rho_;
		/// Number of primary vertices (NPV):
		npv = 0;
		edm::Handle<reco::VertexCollection> pvs;
		iEvent.getByToken(vertexCollection_, pvs);
		for (vector<reco::Vertex>::const_iterator ipv = pvs->begin(); ipv != pvs->end(); ipv++) {
			if (ipv->ndof() > 4 && ipv->isFake() == false) {
				npv += 1;
			}
		}
		
		/// Save event-wide variables:
		branches["event"]["sigma"].push_back(sigma_);             // Provided in the configuration file
//		cout << n_event << endl;
		branches["event"]["nevent"].push_back(n_event);           // Event counter
//		branches["event"]["nevents"].push_back(nevents_);         // Provided in the configuration file
		branches["event"]["w"].push_back(weight_);                // The event weight
		branches["event"]["pt_hat"].push_back(pt_hat);            // Maybe I should take this out of "PF"
		
		// Process each object collection:
		process_jets_pf(iEvent, "ak4", ak4PFCollection_);
		process_jets_pf(iEvent, "ak8", ak8PFCollection_);
		process_jets_pf(iEvent, "ca12", ca12PFCollection_);
		process_jets_gn(iEvent, "ak4", ak4GNCollection_);
		process_jets_gn(iEvent, "ak8", ak8GNCollection_);
		process_jets_gn(iEvent, "ca12", ca12GNCollection_);
		process_jets_maod(iEvent, "ak4", ak4MAODCollection_);
		process_jets_maod(iEvent, "ak8", ak8MAODCollection_);
		process_electrons_pf(iEvent, electronCollection_);
		process_muons_pf(iEvent, muonCollection_);
		process_tauons_pf(iEvent, tauCollection_);
		process_photons_pf(iEvent, photonCollection_);
		
		// Fill ntuple:
		ttrees["events"]->Fill();		// Fills all defined branches.
	}                 // :End in_type == 1
	else {
		cout << "You input an unknown \"in type\": " << in_type_ << endl;
	}
}

// ------------  called once each job just after ending the event loop  ------------
void JetAnalyzer::endJob()
{
//	cout << "END!" << endl;
}

// ------------ method called when starting to processes a run  ------------
void 
JetAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
JetAnalyzer::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
JetAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
JetAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

void
JetAnalyzer::respondToOpenInputFile(edm::FileBlock const& fb)
{
//	cout << "NEW FILE" << endl;
//	cout << fb.fileName() << endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
JetAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(JetAnalyzer);