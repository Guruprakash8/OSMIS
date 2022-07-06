#include <bits/stdc++.h>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

struct nodeElement {
    int index = -1;         // Every node is bound to an index (makes life easier when using adjacency lists)
    string id = "\0";       // node id is stored as a string
    string name = "\0";     
    double lat = 0;         // node Latitude
    double lon = 0;         // node Longitude
};

double haversine(double lat1, double lon1, double lat2, double lon2) {
    // FUnction for computing crow fly distance between two places on earth identified by their latitudes and longitudes
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = (lat1)*M_PI / 180.0;
    lat2 = (lat2)*M_PI / 180.0;
    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c * 1000;          // Returns distance in metres
}

void traverse_xml(const std::string &input_xml, vector<nodeElement> &nodes, map<string, int> &mapIndex, vector<vector<pair<int, double>>> &adj_list, int &num_ways)
{
    // Safe-to-modify copy of input_xml
    vector<char> xml_copy(input_xml.begin(), input_xml.end());
    xml_copy.push_back('\0');

    xml_document<> doc;
    doc.parse<0>(&xml_copy[0]);

    ofstream output;
    output.open("parsed_data.txt");    // Writing into "parsed_data.txt" file
    bool resize_flag = false;          // A flag to resize the adjacency list vector

    xml_node<> *root_node = doc.first_node("osm");
    for (xml_node<> *node = root_node->first_node("node"); node; node = node->next_sibling())
    {
        if (strcmp(node->name(), "node") == 0) {   
            // Traversing the node elments

            // A nodeELement that would be pushed to the vector<nodeELement> nodes
            struct nodeElement temp;
            // string str = node->first_attribute("id")->value();
            temp.id = node->first_attribute("id")->value();          // nodeELemenr id
            temp.lat = stod(node->first_attribute("lat")->value());  // nodeELement latitude
            temp.lon = stod(node->first_attribute("lon")->value());  // nodeELement longitude
            temp.index = nodes.size();                               // nodeElement bound to an index
            bool flag = false;

            if (node->first_node("tag")) {
                for (xml_node<> *tag = node->first_node("tag"); tag; tag = tag->next_sibling("tag")) {
                    string name_holder = tag->first_attribute("k")->value();
                    if (name_holder == "name") {
                        temp.name = tag->first_attribute("v")->value();  // nodeELement name if found is extracted from the "map.osm" file
                        flag = true;
                        break;
                    }
                }
            }

            if (!flag)
                temp.name = "\0";   // Making sure that the name of the nodeELement remains empty incase not found in the "map.osm" file

            // Binding a node id with an index by inserting the pair in the map
            mapIndex.insert(pair<string, int>(temp.id, temp.index));

            // Pushing the nodeELement to the list of nodes
            nodes.push_back(temp);

            // Writing the nodeELement to the output stream, aka, "parsed_data.txt" file 
            output << setprecision(10) << "\t" << nodes.back().id << "\t" << nodes.back().index << "\t" << nodes.back().lat << "\t" << nodes.back().lon << "\t" << nodes.back().name << endl;
        }
        else if (strcmp(node->name(), "way") == 0) {

            // Traversing the way elements

            if (!resize_flag) {
                adj_list.resize(nodes.size());
                resize_flag = true;
            }
            if (node->first_node("nd"))
            {
                for (xml_node<> *nd = node->first_node("nd"); nd->next_sibling("nd"); nd = nd->next_sibling("nd"))
                {
                    string s1 = nd->first_attribute("ref")->value();
                    string s2 = nd->next_sibling()->first_attribute("ref")->value();
                    int index1, index2;
                    auto itr1 = mapIndex.find(s1);
                    if (itr1 != mapIndex.end())
                    {
                        index1 = itr1->second;
                    }
                    auto itr2 = mapIndex.find(s2);
                    if (itr2 != mapIndex.end())
                    {
                        index2 = itr2->second;
                    }

                    // Computing crow-fly distance
                    double dist = haversine(nodes[index1].lat, nodes[index1].lon, nodes[index2].lat, nodes[index2].lon);
                    // Making entries in adjacency list
                    adj_list[index1].push_back(make_pair(index2, dist));
                    adj_list[index2].push_back(make_pair(index1, dist));
                    output << setprecision(10) << num_ways << "\t" << s1 << "\t" << s2 << "\t" << index1 << " " << index2 << " " << endl;
                    output << setprecision(10) << "\t" << nodes[index1].index << "\t" << nodes[index1].id << "\t" << nodes[index1].lat << "\t" << nodes[index1].lon << " " << nodes[index1].name << endl;
                    output << setprecision(10) << "\t" << nodes[index2].index << "\t" << nodes[index2].id << "\t" << nodes[index2].lat << "\t" << nodes[index2].lon << " " << nodes[index2].name << endl;
                    output << setprecision(10) << "\t" << adj_list[index1].back().first << "\t" << adj_list[index1].back().second << endl;
                    output << setprecision(10) << "\t" << adj_list[index2].back().first << "\t" << adj_list[index2].back().second << endl;
                }
            }
            num_ways++;
        }
    }
    output.close();
    cout << "Total number of nodes discovered = " << nodes.size() << endl;   // Printing the number of node elements discovered
    cout << "Total number of ways discovered = " << num_ways << endl;        // Printing the number of ways discovered
}


void searchNode(vector<nodeElement> &nodes) {
    cout << "Enter the name of the node that is to be searched : " << endl;
    string str1, str2;
    getchar();
    getline(cin, str1);
    int N = nodes.size();
    bool flag = false;
    for (int i = 0; i < N; ++i) {
        // Loop that runs over all the nodes with names and tries to match with the input string (Not substring matching or subsequence matching)

        if (nodes[i].name != "\0") {
            str2 = nodes[i].name;
            int count = 0;
            int n = str2.size();
            bool table[n];
            for (int j = 0; j < n; ++j)
                table[j] = false;
            
            for (int j = 0; j < str1.size(); ++j){
                for (int k = 0; k < n; ++k) {
                    if (((str2[k] == str1[j]) || (str2[k] - str1[j] == 32) || (str1[j] - str2[k] == 32)) && table[k] == false) {
                        table[k] = true;
                        count++;
                        break;
                    }
                }
            }
            if (count >= 0.9 * str1.size()) {
                if (!flag) cout << "Match found : " << endl;
                flag = true;
                cout << setprecision(10) << "Node id = " << nodes[i].id << "\t" << nodes[i].name << "\t Latitude = " << nodes[i].lat << "\t Longitude = " << nodes[i].lon << endl;
            }
        }
    }
    if (!flag) {
        cout << "No Match Found" << endl;
    }
}

void printCommands() {
    // Basic Functionalities provided to choose from
    cout << "Enter 0 to exit the app " << endl;
    cout << "Enter 1 to look for a node in the map" << endl;
    cout << "Enter 2 to print the k-closest nodes to a given node" << endl;
    cout << "Enter 3 to find the shortest path between two node elements" << endl;
}

void kClosest(vector<nodeElement>& nodes, map<string, int>& mapIndex) {
    cout << "Enter the id of the node of interest for which k-closest nodes are to be printed : " << endl;
    string s;
    int k;
    cin >> s;
    cout << "Enter the value of k : " << endl;
    cin >> k;

    auto itr = mapIndex.find(s);
    int index = itr->second;

    if (itr != mapIndex.end()){
        vector<pair<double, int>> vec;
        for (int i = 0; i < nodes.size(); ++i){
            vec.push_back(make_pair(haversine(nodes[index].lat, nodes[index].lon, nodes[i].lat, nodes[i].lon), i));
        }
        sort(vec.begin(), vec.end());
        for (int i = 1; i <= k; ++i){
            int j = vec[i].second;
            cout << setprecision(10) << "Distance = " << vec[i].first << " metres \t" << "Node id = " << nodes[j].id << "\t";
            cout << setprecision(10) << "Latitude = " << nodes[i].lat << "\t Longitude = " << nodes[i].lon << endl;
        }
    } else{
        cout << "Invalid node ID has been entered" << endl;
    }
}

void shortestPath(vector<nodeElement> &nodes, map<string, int> &mapIndex, vector<vector<pair<int, double>>> &adj_list) {

    // Dijkstra's ALgorithm

    int parent[nodes.size()];

    for (int i = 0; i < nodes.size(); ++i)
    {
        parent[i] = INT_MAX;
    }
    
    cout << "Enter node id 1 : " << endl;
    string id1;
    cin >> id1;
    cout << "Enter node id 2 : " << endl;
    string id2;
    cin >> id2;

    int start = mapIndex.find(id1)->second;
    int end = mapIndex.find(id2)->second;
    set<pair<double, int>> set;
  
    vector<double> path_distance(nodes.size(), DBL_MAX);
    path_distance[start] = 0;
    set.insert(make_pair(0, start));

    parent[start] = -1;
    
    while (!set.empty())
    {
        pair<double, int> temp = *(set.begin());
        set.erase(set.begin());
        int u = temp.second;
  
        for (int i = 0; i < adj_list[u].size(); ++i)
        {
            int vertex = adj_list[u].at(i).first;
            double weight = adj_list[u].at(i).second;
  
            if (path_distance[vertex] > path_distance[u] + weight)
            {
                if (path_distance[vertex] != DBL_MAX) {
                    set.erase(set.find(make_pair(path_distance[vertex], vertex)));
                }

                path_distance[vertex] = path_distance[u] + weight;
                set.insert(make_pair(path_distance[vertex], vertex));
                
                // Keeping track of the path
                parent[vertex] = u;
            }
        }
    }

    vector<int> path;
    if (path_distance[end] < DBL_MAX) {
        cout << "The distance of shortest path between the nodes " << id1 << " and " << id2 << " is " << path_distance[end] << " metres = " << path_distance[end]/1000 << " kms" << endl;
        int index = end;

        ofstream out;
        out.open("path.txt");

        out << "Start :" << endl;
        while (1)
        {
            path.push_back(index);
            index = parent[index];
            if (index == -1) break;
        }

        for (int i = path.size() - 1; i >= 0; i--)
        {
            out << nodes[path[i]].id << endl;
        }
        out << ": End" << endl;
        out.close();
        
    } else {
        cout << "The nodes " << id1 << " and " << id2 << " are not connected through any of the given ways, hence, shortest path has not been found" << endl;
    }
    
}

int main(void)
{
    xml_document<> doc;
    std::ifstream file("map.osm");
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    const std::string input_xml(buffer.str());

    vector<nodeElement> nodes;                     // Vector of nodeElements (Contains id, name, latituse, longitude and index(bound to))
    map<string, int> mapIndex;                     // Map to bind node ids with the binding indices
    vector<vector<pair<int, double>>> adj_list;    // Adjacency list to represent the graph (pair<binding index, distance>)
    int num_ways = 0;

    traverse_xml(input_xml, nodes, mapIndex, adj_list, num_ways);
    int cmd = -1;

    while (1){
        printCommands();
        cin >> cmd;
        if (cmd == 0) {
            cout << "Session ended " << endl;
            break;
        } else if (cmd == 1) {
            searchNode(nodes);
        } else if (cmd == 2) {
            kClosest(nodes, mapIndex);
        } else if (cmd == 3) {
            shortestPath(nodes, mapIndex, adj_list);
        } else cout << "Invalid Input" << endl;
    }

    return 0;
}
