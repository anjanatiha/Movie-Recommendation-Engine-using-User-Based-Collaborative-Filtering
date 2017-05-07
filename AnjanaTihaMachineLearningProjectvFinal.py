
# coding: utf-8

# In[183]:

import math
import numpy as np
import operator

movie_titles_map = {}
movie_user_map={}
user_movie_map={}
user_avg_rating = {}
cache = {}

def read_movie_title(fname):   
    with open(fname) as f:
        for line in f.readlines():
            parts = [x.strip() for x in line.split(',')]
            movie_id = int(parts[0])
            movie_titles_map[movie_id] = parts[2] + "("+ parts[1]+")"    

def get_ratings_map(fname):
    with open(fname) as f:
        for line in f.readlines():
            parts = [x.strip() for x in line.split(',')]
            movie_title_id = int(float(parts[0]))
            user_id = int(float(parts[1]))
            rating = float(parts[2])
            if user_id not in user_movie_map:
                user_movie_map[user_id]={}
            user_movie_map[user_id][movie_title_id] = rating
            
            if movie_title_id not in movie_user_map:
                movie_user_map[movie_title_id] = []
            movie_user_map[movie_title_id].append(user_id)

def get_user_avg_rating():
    for user in user_movie_map:
        sum = 0
        i=0
        for movie in user_movie_map[user]:
            sum = sum + float(user_movie_map[user][movie])
            i = i + 1 
        avg = sum/i
        user_avg_rating[user]=avg


# In[184]:

movie_titles_filename="movie_titles.txt"
ratings_filename="ratings.txt"
read_movie_title(movie_titles_filename)
get_ratings_map(ratings_filename)
get_user_avg_rating()


# In[185]:

def get_user_corr(active_user):
    user_correlation = {}
    for user in user_movie_map:
        if user!=active_user:
            nominator = 0
            sum_vaj_diff = 0
            sum_vij_diff = 0
            for movie in user_movie_map[active_user]:
                if movie in user_movie_map[user]:                
                    nominator +=  (user_movie_map[active_user][movie] - user_avg_rating[active_user]) * (user_movie_map[user][movie] - user_avg_rating[user])
                    sum_vaj_diff +=  np.power(user_movie_map[active_user][movie] - user_avg_rating[active_user], 2)
                    sum_vij_diff +=  np.power(user_movie_map[user][movie] - user_avg_rating[user], 2)
            denominator= np.sqrt(sum_vaj_diff * sum_vij_diff)  
            if denominator!=0:
                user_correlation[user] = nominator/denominator
    return user_correlation


# In[186]:

def recommendation(active_user, K):
    if active_user not in cache:
        user_correlation = get_user_corr(active_user)
        predicted_rating ={}
        for movie in movie_titles_map:
            temp_rating = 0
            if movie in movie_user_map:
                for user in movie_user_map[movie]:
                    if user in user_correlation:
                        temp_rating +=user_correlation[user]*(user_movie_map[user][movie]- user_avg_rating[user])
            predicted_rating[movie] = temp_rating

        predicted_rating = sorted(predicted_rating.items(), key=lambda kv: kv[1], reverse=True)
        cache[active_user] = predicted_rating
    else:
        predicted_rating= cache[active_user]
        
    recommended_movies = predicted_rating[:K]
    for movie in recommended_movies:
        print(movie_titles_map[movie[0]])
        


# In[189]:

active_user=2482502
K=5
recommendation(active_user, K)

