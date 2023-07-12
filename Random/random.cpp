#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <vector>
#include <chrono>

using namespace boost;
using namespace std;
using namespace std::chrono;


#define N 1000 /*initialization of nodes for graph*/
#define COST_GEN_RANGE 10 /*Capacity generator upper limit*/


struct NodeProperty{
	int pred;
	int visited;
};

struct EdgeProperty {
	int value; 
};

typedef adjacency_list<vecS, vecS, undirectedS, NodeProperty, EdgeProperty> Graph;
typedef graph_traits<Graph>::edge_parallel_category disallow_parallel_edge_tag;
typedef graph_traits<Graph>::vertex_descriptor vertex_d;
typedef graph_traits<Graph>::edge_descriptor edge_d;
typedef graph_traits<Graph>::vertex_iterator vertex_t;
typedef graph_traits<Graph>::edge_iterator edge_t;
typedef graph_traits<Graph>::out_edge_iterator out_edge_t;

typedef property_map<Graph, int NodeProperty::*>::type node_property_map;
typedef property_map<Graph, int EdgeProperty::*>::type edge_property_map;

pair<vector<vertex_d>, int> minimum_cut(vertex_d s, vertex_d t, edge_property_map& val, Graph& G);
vertex_d locate(Graph& sep_subtree, Graph& G, vertex_d p, edge_property_map& mvm, edge_property_map& Gvals);
vector<vertex_d> comp_cut_set(vector<vertex_d> cut_set, vector<vertex_d> N_set);
vector<vertex_d> get_set_N(Graph& G);
void init(Graph& graph, edge_property_map& epm);

int main() {
	/*initialization of clock using chrono library*/
	auto start = high_resolution_clock::now();
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	/*end of timer initialization*/

	Graph G(N); /*create a graph that has N nodes*/
	Graph seperator_tree(N); /*create a graph that has N nodes*/

	/*Integers counter, attempts and end will be used as checkers to ensure that the random graph will be generated correctly*/
	int counter = 0;
	int attempts = 0;
	int end = 0;

	vertex_t vi, vi_from, vi_end;

	srand(time(0)); /*change the seed according to current time*/
	for (tie(vi_from, vi_end) = vertices(G); vi_from != vi_end; vi_from++) {
		counter = 0;
		attempts = 0;
		
		end = rand() % 2 + 1; /*end here is used as a limit that controls how many edges will be created for a specific node. There can be max 2 edges right now*/
		while (counter != end) { /*counter controls the number of edges that have already been generated for a specific node*/
			if (attempts == 10) break; /*here the variable attempts define a limit that if while passes, it has to exit. This is used to avoid infinite loops in case of a completely connected graph*/
			vertex_d vi_to = rand() % N; /*pick a random node*/
			while (vi_to == *vi_from) vi_to = rand() % N; /*check whether start node and end node are the same and if so change the end node*/
			if (!edge(*vi_from, vi_to, G).second) { /*if there is no edge connecting the start and end node then create an edge*/
				add_edge(*vi_from, vi_to, G);
				counter++;
				
			}
			attempts++;
		}
	}
	cout << "Number of Nodes = " << num_vertices(G) << endl;
	cout << "Number of edges = " << num_edges(G) << endl;

	
	edge_property_map value_map = get(&EdgeProperty::value, G); /*value_map holds information about the capacities of graph G*/

	edge_property_map min_value_map = get(&EdgeProperty::value, seperator_tree); /*min_value_map holds information about the capacities of the seperator_tree T*/

	init(G, value_map); /*Function to initialize all capacities of edges*/

	edge_t ei, ei_end;

	/*this comment below is used as a debugging tool which shows the generated graph*/
	/*for (tie(ei, ei_end) = edges(G); ei != ei_end; ei++) {
		std::cout << "we got an edge linking nodes " << source(*ei, G) + 1 << " and " << target(*ei, G) + 1 << " with value of " << value_map[*ei] << endl;
	}*/
	
	vertex_d k;
	
	start = high_resolution_clock::now(); /*clock begins counting*/

	/*This for is the heart of the program. It calls the essential functions locate and minimum_cut that create the seperator tree*/
	for (int i = 0; i < N; i++) {
		if (i == 0) continue; /*obviously at the start the seperator tree is considered empty so we simply add the first node in the tree*/
		else {
			k = locate(seperator_tree, G, i, min_value_map, value_map);	/*recursively add all nodes in the seperator tree and create their corresponding edges*/
			min_value_map[edge(i, k, seperator_tree).first] = minimum_cut(i, k, value_map, G).second; /*Now that the edges are created we update their capacities to be equal to the minimum cut of the start and end nodes*/
		}
	}
	/*we use the minimum_cuts variable to describe every edge within the seperator tree. In other words we save the seperator tree within this variable in the form of a vector*/
	vector<pair<edge_d,int>> minimum_cuts;
	pair<edge_d, int> result;
	for (tie(ei, ei_end) = edges(seperator_tree); ei != ei_end; ei++) {
		result.first = *ei;
		result.second = min_value_map[*ei];
		minimum_cuts.push_back(result);
	}
	/*The comments below are used as a debugging tool to show on screen both the seperator tree and all pairs minimum cuts*/

	/*for (tie(ei, ei_end) = edges(seperator_tree); ei != ei_end; ei++) {
		std::cout << "there is an edge linking " << source(*ei, seperator_tree) + 1 << " and " << target(*ei, seperator_tree) + 1 << " and has a minimum cut of " << min_value_map[*ei] << endl;
	}*/

	/*for (int i = 0; i < N; i++) {
		for (int j = i; j < N; j++) {
			if (i != j) std::cout << "Pair " << i + 1 << " and " << j + 1 << " has a minimum cut value of " << minimum_cut(i, j, min_value_map, seperator_tree).second << endl;
		}
	}*/

	stop = high_resolution_clock::now(); /*stop clock counting*/
	duration = duration_cast<microseconds>(stop - start); /*return the total time*/
	cout << "Total time -> " << (double)duration.count() / 1000000 << " seconds" << endl; /*print total time in seconds format*/
	return 0;
}

/*Here we create random capacities and add them to the graph edges*/
void init(Graph& graph, edge_property_map& epm) {
	edge_t ei, ei_end;
	vertex_t vi, vi_end;
	srand(time(0));
	for (tie(ei, ei_end) = edges(graph); ei != ei_end; ei++) {
		epm[*ei] = rand() % COST_GEN_RANGE + 1;
	}
}


pair<vector<vertex_d>,int> minimum_cut(vertex_d s, vertex_d t, edge_property_map& val, Graph& G) {
	out_edge_t ei, ei_end; /*checks for the edges that come out from a specific node*/
	vertex_t vi, vi_end;
	node_property_map pred = get(&NodeProperty::pred, G); /*create a predecessor map*/
	node_property_map visited = get(&NodeProperty::visited, G); /*create a visited map*/
	int spread = 1; /*spread variable is used as a search limit. If it's 1 it checks for the neighboor nodes of starting node, if it's 2 it checks for the neighboors of the neighboors of the starting node and so on*/
	visited[s] = 1; /*obviously we mark the node that we start from as visited*/
	visited[t] = 1; /*we also mark this starting node as visited since we also check for a cut that comes from this node*/

	int min_cut = INT_MAX; /*initialization with the maximum integer*/
	int sum = 0;

	vector<vertex_d> source_adj, target_adj, cut_set_A; /*these vectors will be used to store information about the neighbooring nodes in the searching process. Cut_set_A is used to store the first set after the cut*/

	vector<vertex_d> temp_source;
	
	for (tie(ei, ei_end) = out_edges(s, G); ei != ei_end; ei++) { /*for all edges that come out of node s*/
		if (target(*ei, G) != t) {
			source_adj.push_back(target(*ei, G)); /*store neighboor nodes*/
		}
		sum += val[*ei]; /*store the cost of the cut as the sum of the capacities that are in the cut*/
	}
	if (sum < min_cut) { /*store the minimum cut in the first set*/
		cut_set_A.clear();
		min_cut = sum;
		cut_set_A.push_back(s);
	}
	vertex_d next_s;
	int temp = sum;
	for (int i = 0; i < source_adj.size(); i++) { /*for each neighbooring node*/
		next_s = source_adj[i]; /*check the other nodes*/
		pred[next_s] = s;
		/*initialize the visited of all other nodes as "not visited"*/
		for (tie(vi, vi_end) = vertices(G); vi != vi_end; vi++) {
			if (*vi != s && *vi != t) visited[*vi] = 0;
		}
		visited[next_s] = 1; /*mark current node as visited*/

		for (int j = 0; j < spread; j++) { /*for each neighboor (if spread = 1 then we only check for current node next_s)*/
			temp = temp - val[edge(pred[next_s], next_s, G).first]; /*update the cut value correctly*/
			/*for each neighboor of next_s node*/
			for (tie(ei, ei_end) = out_edges(next_s, G); ei != ei_end; ei++) {

					if (out_degree(source(*ei, G), G) >= 2) {
						
						if (visited[target(*ei, G)] == 0) {
							temp_source.push_back(target(*ei, G));
						}
					}
					else if (out_degree(source(*ei, G), G) == 1){
						break;
					}
					if (target(*ei, G) != s)temp += val[*ei];
				
			}
			if (temp_source.empty()) {
				break;
			}
			pred[*temp_source.begin()] = next_s;
			next_s = *temp_source.begin();
			visited[next_s] = 1;
			/*check if this cut value is minimum and if yes update the cut_set_A variable so that it contains the set of nodes that are cut from graph G*/
			if (temp < min_cut) {
				cut_set_A.clear();
				min_cut = temp;
				vertex_d prev_s = next_s;

				for (int k = 0; k < temp_source.size(); k++) {
					cut_set_A.push_back(pred[prev_s]);
					prev_s = pred[prev_s];
				}
				cut_set_A.push_back(s);
			}
			temp_source.clear();
			
		}
		
		temp = sum;
	}
	/*the rest below are exactly the same as with node s but this time for node t instead.*/
	sum = 0;
	for (tie(ei, ei_end) = out_edges(t, G); ei != ei_end; ei++) {
		if (target(*ei, G) != s) {
			target_adj.push_back(target(*ei, G));
			
		}
		sum += val[*ei];
	}

	if (sum < min_cut) {
		cut_set_A.clear();
		min_cut = sum;
		cut_set_A.push_back(t);
	}
	temp = sum;
	vertex_d next_t;
	vector<vertex_d> temp_target;
	for (int i = 0; i < target_adj.size(); i++) {
		next_t = target_adj[i];
		pred[next_t] = t;

		for (tie(vi, vi_end) = vertices(G); vi != vi_end; vi++) {
			if (*vi != s && *vi != t) visited[*vi] = 0;
		}
		visited[next_t] = 1;

		for (int j = 0; j < spread; j++) {
			temp = temp - val[edge(pred[next_t], next_t, G).first];

			for (tie(ei, ei_end) = out_edges(next_t, G); ei != ei_end; ei++) {

				if (out_degree(source(*ei, G), G) >= 2) {

					if (visited[target(*ei, G)] == 0) {
						temp_target.push_back(target(*ei, G));
					}
				}
				else if (out_degree(source(*ei, G), G) == 1) {
					break;
				}
				if (target(*ei, G) != t)temp += val[*ei];

			}
			if (temp_target.empty()) {
				break;
			}
			
			pred[*temp_target.begin()] = next_t;
			next_t = *temp_target.begin();
			visited[next_t] = 1;

			if (temp < min_cut) {
				cut_set_A.clear();
				min_cut = temp;
				vertex_d prev_t = next_t;
				
				for (int k = 0; k < temp_target.size(); k++) {
					cut_set_A.push_back(pred[prev_t]);
					prev_t = pred[prev_t];
				}
				cut_set_A.push_back(t);
			}
			
			temp_target.clear();

		}

		temp = sum;
	}

	/*we create a pair of a vertex vector and and integer that together create the results that are returned by the function*/
	pair<vector<vertex_d>, int> res;
	res.first = cut_set_A; /*right now cut_set_A should contain the subset of nodes that are cut from graph G*/
	res.second = min_cut; /*min_cut should contain the correct value of the minimum cut that occured*/
	return res;
}

/*This function simply returns the whole set of nodes for graph G*/
vector<vertex_d> get_set_N(Graph& G) {
	vertex_t vi, vi_end;
	vector<vertex_d> set;
	for (tie(vi, vi_end) = vertices(G); vi != vi_end; vi++) set.push_back(*vi);
	return set;
}
/*This function simply returns the subset of the nodes that remained within G after the minimum cut. In other words it creates the complinent subset of cut_set_A that is created within minimum_cut function*/
vector<vertex_d> comp_cut_set(vector<vertex_d> cut_set, vector<vertex_d> N_set){
	vector<vertex_d> comp_set;
	bool found;
	for (int i = 0; i < N_set.size(); i++) {
		found = false;
		for (int j = 0; j < cut_set.size(); j++) {
			if (N_set[i] == cut_set[j]) found = true;
		}
		if (!found) comp_set.push_back(N_set[i]);
	}
	return comp_set;
}

vertex_d locate(Graph& sep_subtree, Graph& G, vertex_d p, edge_property_map& mvm, edge_property_map& Gvals){
	edge_t ei, ei_end;
	vertex_d k;
	vertex_d singleton = 0;
	pair<vector<vertex_d>, int> cut;
	vector<vertex_d> comp;
	int min = INT_MAX;
	char direction; /*This variable shows in which direction of the cut within the seperator tree lies our node k*/
	vertex_d a=0, b=0, a_cand=0, b_cand=0; /*vertex a and b are the nodes that have the minimum cut within the seperator tree*/
	int found = 0;
	int check = 0;
	int threshold = 0;
	int attempts = 0; /*since we use an infinite loop we use the variable attempts to force exit the while loop in case of an undefined random state*/

		while (1) {
			if (p==1) break; /*obviously if we are trying to add the second node within the seperator subtree then k should be the first node within the tree since it's a singleton*/
			direction = 'n'; /*initialize the direction with 'nothing'*/
			
			min = INT_MAX;
			found = 0;
			check = 0;
			/*we locate the minimum a-b minimum cut candidates within this for*/
			for (tie(ei, ei_end) = edges(sep_subtree); ei != ei_end; ei++) {
				if (mvm[*ei] < min && mvm[*ei] >= threshold ) {
					if (source(*ei, sep_subtree) != a || target(*ei, sep_subtree) != b) {
						min = mvm[*ei];
						check = 1;
						a_cand = source(*ei, sep_subtree);
						b_cand = target(*ei, sep_subtree);
					}
				}
			}
			/*we update threshold to avoid falling into infinite loops but at the same time check every possible edge even if more than two edges have the same capacity*/
			if (threshold == min) attempts++;
			threshold = min;
			if (attempts == 2) { /*if the program is stuck more than 2 times looking at the same edge then force it to move on by increasing threshold by one*/
				attempts = 0;
				threshold++;
			}
			a = a_cand;
			b = b_cand;

			cut = minimum_cut(a, b, Gvals, G); /*return the value of the minimum cut and the cut_set_A between nodes a and b*/
			comp = comp_cut_set(cut.first, get_set_N(G)); /*find the complinent subset of cut_set_A*/
			
			/*the rest of this code checks where k is within the a-b cut and repeats the while loop until it gets into a singleton set*/
			for (int i = cut.first.size() - 1; i >= 0; i--) {
				if (cut.first[i] == a) direction = 'a';
				else if (cut.first[i] == b) direction = 'b';
			}

			for (int i = cut.first.size() - 1; i >= 0; i--) {
				if (cut.first[i] == p) found = 1;
			}
			if (found == 1 && direction == 'a') {
				if (cut.first.size() == 2) {
					singleton = a;
					break;
				}

			}
			else if (found == 1 && direction == 'b') {
				if (cut.first.size() == 2) {
					singleton = b;
					break;
				}
				

			}
			else if (found == 0 && direction == 'a') {
				if (out_degree(b, sep_subtree) == 1) {
					singleton = b;
					break;
				}
				if (check == 0) {
					singleton = b;
					break;
				}
				

			}
			else if (found == 0 && direction == 'b') {
				if (out_degree(a, sep_subtree) == 1) {
					singleton = a;
					break;
				}
				if (check == 0) {
					singleton = a;
					break;
				}
				

			}
			
		}
		k = singleton; /*k should now contain the correct singleton node*/
		edge_d e = add_edge(p, k, sep_subtree).first; /*we link singleton node k with the node p that we were trying to add in our seperator tree*/
	return k; /*we return the singleton node k*/
}
