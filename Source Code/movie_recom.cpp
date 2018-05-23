//============================================================================
// Name        : Movie Recommendation.cpp
// Author      : Anjana Tiha
// Version     : 1
// Copyright   : MIT
// Description : Movie Recommendation using Netflix Data
//============================================================================

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include<unordered_map>
#include<vector>
#include <algorithm>
#include<cmath>
using namespace std;

struct movie_rate{
	int movie_id;
	float rating;
};

bool compare_movie (const movie_rate& lhs,const movie_rate& rhs){
	return lhs.rating>rhs.rating;
}

unordered_map<int, unordered_map<int, float>> rating_map;
unordered_map<int, vector<int>> movie_user;
unordered_map<int, float> avg_rating;
unordered_map<int, string> movie_map;

unordered_map<int, vector<movie_rate>> cache;

bool read_movietitle(string filename) {
	fstream movie_title_file(filename, ios::in);
	if (!movie_title_file.is_open())
		return false;

	int id_int;
	string ID, year, title;
	while (getline(movie_title_file, ID, ',')) {
		getline(movie_title_file, year, ',');
		getline(movie_title_file, title);

		id_int = stoi(ID);
		movie_map[id_int] = title + "(" + year + ")";
	}

	movie_title_file.close();
	return true;
}

bool read_rating(string filename) {
	fstream rating_file(filename, ios::in);
		if (!rating_file.is_open())
			return false;

		int user_id, movie_id,rating;
		string movie_id_str, user_id_str, rating_str;
		while (getline(rating_file, movie_id_str, ',')) {
			getline(rating_file, user_id_str, ',');
			getline(rating_file, rating_str);
			user_id = stoi(user_id_str);
			movie_id = stoi(movie_id_str);
			rating=stof(rating_str);

			/*if(rating_map.find (user_id) == rating_map.end()){
        	  unordered_map<int, float> temp;
        	  rating_map[user_id]=temp;
			}*/

          rating_map[user_id][movie_id]=rating;

          /*if (movie_user.find (movie_id) == movie_user.end()){
        	  vector<int> temp;
        	  movie_user[movie_id]=temp;
          }*/

          movie_user[movie_id].push_back(user_id);
	}

	rating_file.close();
	return true;
}

void user_rating_avg(){
	for (auto& user_it : rating_map){
		float sum=0;
		int count = 0;
		for (auto& user_movie_it : user_it.second){
			sum +=user_movie_it.second;
			++count;
		}
		avg_rating[user_it.first]=sum/count;
	}
}

unordered_map<int, float> get_correlation(int active_user){
	unordered_map<int, float> corr_map;
	unordered_map<int, float>& active_user_movie_rate=rating_map[active_user];

	for(auto& user_it: rating_map){
		int user_id=user_it.first;

		if(user_id==active_user)
			continue;

		unordered_map<int, float>& user_movie_rate=user_it.second;

		float nominator = 0;
		float sum_vaj_diff = 0;
		float sum_vij_diff = 0;

		for(auto& movie_it: active_user_movie_rate){
			int movie_id = movie_it.first;

			if (user_movie_rate.find(movie_id) == user_movie_rate.end())
				 continue;

			nominator +=  (rating_map[active_user][movie_id] - avg_rating[active_user]) * (rating_map[user_id][movie_id] - avg_rating[user_id]);
			sum_vaj_diff +=  pow(rating_map[active_user][movie_id] - avg_rating[active_user], 2);
			sum_vij_diff +=  pow(rating_map[user_id][movie_id] - avg_rating[user_id], 2);

		}

        float denominator= sqrt(sum_vaj_diff * sum_vij_diff);
        if(denominator!=0)
        	corr_map[user_id] = nominator/denominator;
	}

	return corr_map;
}


void movie_recommendation(int active_user, int K){
	vector<movie_rate>* predicted_rating=NULL;

	if(cache.find(active_user)==cache.end()){
		unordered_map<int, float> corr_active = get_correlation(active_user);
		predicted_rating=new vector<movie_rate>();

		for(auto& movie_it : movie_map){
			float pred_rating = 0;
			int movie_id=movie_it.first;

			if(movie_user.find(movie_id)!=movie_user.end()){

				vector<int>& user_list=movie_user[movie_id];

				for(int user_id : user_list){
					if (corr_active.find(user_id) == corr_active.end())
						continue;

					pred_rating +=corr_active[user_id]*(rating_map[user_id][movie_id]- avg_rating[user_id]);
				}
			}

			movie_rate m;
			m.movie_id=movie_id;
			m.rating=pred_rating;

			predicted_rating->push_back(m);

		}
		sort(predicted_rating->begin(), predicted_rating->end(),compare_movie);

		cache[active_user]=*predicted_rating;
		corr_active.clear();
	}else{
		predicted_rating=&cache[active_user];
	}

	cout<<"\n"<<K <<" recommended movies for user : "<<active_user<<"\n";
	for(int i=0;i<K;++i)
		cout<<movie_map[predicted_rating->at(i).movie_id]<<"\n";

}

void clear_all(){
	avg_rating.clear();
	movie_map.clear();

	for(auto& it:rating_map)
		it.second.clear();

	rating_map.clear();

	for(auto& it:movie_user)
		it.second.clear();

	movie_user.clear();

	for(auto& it:cache)
		it.second.clear();

	cache.clear();
}

int main(int argc, char ** argv) {

	string movie_title_file="D:/Academic/Spring 2017/Machine Learning/Machine Learning HomeWork/project/netflix/movie_titles.txt";
	string rating_file="D:/Academic/Spring 2017/Machine Learning/Machine Learning HomeWork/project/netflix/ratings.txt";

	if(argc==3){
		movie_title_file=argv[1];
		rating_file=argv[2];

	}else{
		cout<<"File name not given. Trying to read file from executable location.\n";
	}

	cout<< "Reading file \""<<movie_title_file<<"\" ...\n";
	if(read_movietitle(movie_title_file)==false){
		cout << movie_title_file<<" not found!\n";
		return 1;
	}

	cout<< "Reading file \""<<rating_file<<"\" ...\n";
	if(read_rating(rating_file)==false){
		cout << rating_file<<" not found!\n";
		return 1;
	}

	cout<< "Computing User average rating.... \n";
	user_rating_avg();

	int user_id, K;
	while(true){
		cout<<"Please Enter UserID (or enter -1 to exit): ";
		cin>>user_id;

		if(user_id==-1){
			clear_all();
			break;
		}

		if(rating_map.find(user_id)==rating_map.end())
			cout<<"UserID not found, Please enter valid UserID\n";
		else{
			cout<<"Please Enter Number of Recommendation: ";
			cin>>K;

			if(K>0)
				movie_recommendation(user_id, K);
			else
				cout<<"Number of recommendation must be greater than 0\n";

		}

		cout<<"\n";
	}

	return 0;
}
