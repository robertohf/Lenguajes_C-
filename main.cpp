#include <cpprest/http_listener.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <stdio.h>
#include <Magick++.h>
#include "base64.h"
#pragma comment(lib, "cpprest110_1_1")
 
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace web::http::client; 
using namespace Magick; 
 
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <regex>
using namespace std;
 
#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"
 
map<utility::string_t, utility::string_t> dictionary;
 
/* handlers implementation */

void ejercicio1(http_request request)
{
  json::value json = request.extract_json(true).get();
  string origen = json.at("origen").as_string();
  origen = regex_replace(origen, regex(" "), "+");
  string destino = json.at("destino").as_string();
  destino = regex_replace(destino, regex(" "), "+");

  http_client client(U("https://maps.googleapis.com"));
  uri_builder builder(U("/maps/api/directions/json"));
  builder.append_query(U("origin"), U(origen));
  builder.append_query(U("destination"), U(destino));
  builder.append_query(U("key"), U("AIzaSyAzzrnc71pLvEvOdY322DQwwbUsFQZT7Vg"));
  http_response response = client.request(methods::GET, builder.to_string()).get();
  json::value maps = response.extract_json().get();
  json::array steps =  maps.at("routes").as_array()[0].at("legs").as_array()[0].at("steps").as_array();

  string response_json = "{\"ruta\":[";
  for(int c = 0; c < steps.size(); c++){
    response_json = response_json + "{\"lat\":";
    response_json = response_json + steps[c].at("start_location").at("lat").serialize();
    response_json = response_json + ", \"lon\":";
    response_json = response_json + steps[c].at("start_location").at("lng").serialize();
    response_json = response_json + "}, ";
    if(c == steps.size() - 1){
      response_json = response_json + "{\"lat\":";
      response_json = response_json + steps[c].at("end_location").at("lat").serialize();
      response_json = response_json + ", \"lon\":";
      response_json = response_json + steps[c].at("end_location").at("lng").serialize();
      response_json = response_json + "}";
    }
  }
  response_json = response_json + "]}";

  json::value *answer = new json::value(response_json, false);
  request.reply(status_codes::OK, *answer);
}

void ejercicio2(http_request request)
{
  json::value json = request.extract_json(true).get();
  string origen = json.at("origen").as_string();
  origen = regex_replace(origen, regex(" "), "+");

  http_client client(U("https://maps.googleapis.com"));
  uri_builder builder(U("/maps/api/geocode/json"));
  builder.append_query(U("address"), U(origen));
  builder.append_query(U("key"), U("AIzaSyDlWabEzv6sC9AW1F_C1rc_nOz9o2nm0Bg"));
  http_response response = client.request(methods::GET, builder.to_string()).get();
  json::value maps = response.extract_json().get();
  string lat = maps.at("results").as_array()[0].at("geometry").at("location").at("lat").serialize();
  string lng = maps.at("results").as_array()[0].at("geometry").at("location").at("lng").serialize();
  string latlng = lat + "," + lng;

  http_client client1(U("https://maps.googleapis.com"));
  uri_builder builder1(U("/maps/api/place/nearbysearch/json"));
  builder1.append_query(U("location"), U(latlng));
  builder1.append_query(U("radius"), U("1000"));
  builder1.append_query(U("type"), U("restaurant"));
  builder1.append_query(U("key"), U("AIzaSyAp0wmWixdzDo3MBI7TIY1XN4okirXUeYM"));
  http_response response1 = client1.request(methods::GET, builder1.to_string()).get();
  json::value maps1 = response1.extract_json().get();

  json::array restaurants = maps1.at("results").as_array();

  string response_json = "{\"restaurantes\":[";
  for(int c = 0; c < restaurants.size(); c++){
    response_json = response_json + "{\"nombre\":\"";
    response_json = response_json + restaurants[c].at("name").as_string();
    response_json = response_json + "\", ";
    response_json = response_json + "\"lat\":";
    response_json = response_json + restaurants[c].at("geometry").at("location").at("lat").serialize();
    response_json = response_json + ", ";
    response_json = response_json + "\"lon\":";
    response_json = response_json + restaurants[c].at("geometry").at("location").at("lng").serialize();
    if(c == restaurants.size() - 1){
      response_json = response_json + "}";
    }else{
      response_json = response_json + "}, ";
    }
  }
  response_json = response_json + "]}";
  json::value *answer = new json::value(response_json, false);
  request.reply(status_codes::OK, *answer);
}

void ejercicio3(http_request request)
{
  json::value json = request.extract_json(true).get();
  string name = json.at("nombre").as_string();
  string data = json.at("data").as_string();

  string decoded = base64_decode(data);
  ofstream out("temp.bmp");
  out << decoded;
  out.close();

  Image img;
  img.read("temp.bmp");

  for(int x = 0; x < img.columns(); x++){
    for(int y = 0; y < img.rows(); y++){
      ColorRGB oldpixel = img.pixelColor(x, y);
      double red = oldpixel.red();
      double green = oldpixel.green();
      double blue = oldpixel.blue();
      double lumi = 0.21 * red + 0.72 * green + 0.07 * blue;
      ColorRGB newpixel(lumi, lumi, lumi);
      img.pixelColor(x, y, newpixel);
    }
  }

  img.write("newimg.bmp");
  ifstream in("newimg.bmp");
  string to_encode((istreambuf_iterator<char>(in)), (istreambuf_iterator<char>()));
  in.close();
  string encoded = base64_encode(to_encode.c_str(), to_encode.size());
  string s = name.substr(0, name.size() -4);
  string response_json = "{\"nombre\":\"" + s + "(blanco y negro).bmp\", \"data\": \"" + encoded + "\"}";
  json::value *answer = new json::value(response_json, false);
  request.reply(status_codes::OK, *answer);
}

void ejercicio4(http_request request)
{
  json::value json = request.extract_json(true).get();
  string name = json.at("nombre").as_string();
  string data = json.at("data").as_string();
  int alto = json.at("tamaño").at("alto").as_integer();
  int ancho = json.at("tamaño").at("ancho").as_integer();

  string decoded = base64_decode(data);
  ofstream out("temp.bmp");
  out << decoded;
  out.close();

  Image img;
  img.read("temp.bmp");
  Image small_img;
  small_img.size(to_string(ancho) + "x" +to_string(alto));
  small_img.magick("RGB");

  int height = img.rows();
  int width = img.columns();

  double divx = (double)width/(double)ancho;
  double divy = (double)height/(double)alto;

  for(int x = 0; x < small_img.columns(); x++){
    for(int y = 0; y < small_img.rows(); y++){
      ColorRGB pixel = img.pixelColor((int)x * divx,(int)y * divy);
      small_img.pixelColor(x, y, pixel);
    }
  }

  small_img.write("newimg.bmp");
  ifstream in("newimg.bmp");
  string to_encode((istreambuf_iterator<char>(in)), (istreambuf_iterator<char>()));
  string encoded = base64_encode(to_encode.c_str(), to_encode.size());
  in.close();
  string s = name.substr(0, name.size() -4);
  string response_json = "{\"nombre\":\"" + s + "(reducido).bmp\", \"data\": \"" + encoded + "\"}";
  json::value *answer = new json::value(response_json, false);
  request.reply(status_codes::OK, *answer);
}
 
int main(int argc, char ** argv)
{
  InitializeMagick(*argv);
   http_listener listener1("http://localhost:8080/ejercicio1");
   http_listener listener2("http://localhost:8080/ejercicio2");
   http_listener listener3("http://localhost:8080/ejercicio3");
   http_listener listener4("http://localhost:8080/ejercicio4");
 
   listener1.support(methods::POST, ejercicio1);
   listener2.support(methods::POST, ejercicio2);
   listener3.support(methods::POST, ejercicio3);
   listener4.support(methods::POST, ejercicio4);
 
  try
  {
    listener1.open().wait();
    listener2.open().wait();
    listener3.open().wait();
    listener4.open().wait();

    while (true);
  }
  catch (exception const & e)
  {
    wcout << e.what() << endl;
  }

  return 0;
}