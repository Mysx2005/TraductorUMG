#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// --- REEMPLAZA ESTO CON TU CLAVE DE GOOGLE ---
const string MI_API_KEY = "AIzaSyBOfIq453ZzyFIDJXbFOPmP4CUthlGfGDs"; 

// Función para manejar la respuesta de la API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Función que se comunica con Google Translate
string traducir(string texto, string lenguajeDestino) {
    CURL* curl;
    CURLcode res;
    string respuestaJson;

    curl = curl_easy_init();
    if(curl) {
        // Escapar caracteres especiales en el texto (como espacios)
        char* textoEscapado = curl_easy_escape(curl, texto.c_str(), texto.length());
        
        string url = "https://translation.googleapis.com/language/translate/v2?q=" + 
                     string(textoEscapado) + "&target=" + lenguajeDestino + "&key=" + MI_API_KEY;
        
        curl_free(textoEscapado);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respuestaJson);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res == CURLE_OK) {
            auto j = json::parse(respuestaJson);
            if (j.contains("data")) {
                return j["data"]["translations"][0]["translatedText"];
            }
        }
    }
    return "[Error de conexion o API Key invalida]";
}

// Función para mostrar sugerencias basadas en el archivo del usuario
void mostrarSugerencias(string usuario) {
    string ruta = "sesiones/" + usuario + ".txt";
    ifstream archivo(ruta);
    map<string, int> historial;
    string palabra;
    int cantidad;

    if (archivo.is_open()) {
        while (archivo >> palabra >> cantidad) {
            historial[palabra] = cantidad;
        }
        archivo.close();
    }

    if (!historial.empty()) {
        cout << "\n--- Sugerencias para " << usuario << " (mas buscadas) ---" << endl;
        // Pasar a vector para ordenar por cantidad
        vector<pair<int, string>> ordenado;
        for (auto const& [pal, cant] : historial) ordenado.push_back({cant, pal});
        sort(ordenado.rbegin(), ordenado.rend());

        for (int i = 0; i < min((int)ordenado.size(), 3); i++) {
            cout << " > " << ordenado[i].second << " (" << ordenado[i].first << " veces)" << endl;
        }
        cout << "---------------------------------------------" << endl;
    }
}

// Función para guardar la palabra en el archivo del usuario
void registrarBusqueda(string usuario, string palabra) {
    string ruta = "sesiones/" + usuario + ".txt";
    map<string, int> historial;
    string p;
    int c;

    // Leer historial actual
    ifstream entrada(ruta);
    while (entrada >> p >> c) historial[p] = c;
    entrada.close();

    // Aumentar contador
    historial[palabra]++;

    // Guardar actualizado
    ofstream salida(ruta);
    for (auto const& [pal, cant] : historial) {
        salida << pal << " " << cant << endl;
    }
    salida.close();
}

int main() {
    string usuario, palabra, idioma;

    cout << "ID de Usuario (para cargar su sesion): ";
    cin >> usuario;

    while (true) {
        mostrarSugerencias(usuario);
        
        cout << "\nPalabra a traducir (o 'salir'): ";
        cin >> palabra;
        if (palabra == "salir") break;

        cout << "Idioma destino (en, fr, it, de): ";
        cin >> idioma;

        string resultado = traducir(palabra, idioma);
        cout << "\nTRADUCCION: " << resultado << endl;

        registrarBusqueda(usuario, palabra);
        cout << "\n=============================================" << endl;
    }

    return 0;
}