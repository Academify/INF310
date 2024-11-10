#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <chrono>

using namespace std;

mutex mux, weight_mux, barrier_mux;
condition_variable cv, barrier_cv;

int weightLimit;
int currentWeight = 0;
int timesRoll;
int customersNumber;
int customersRollCompleted = 0;

void customerConsume(int peso_cliente, int id) {
    for (int i = 0; i < timesRoll; ++i) {
        
        unique_lock<mutex> lock(weight_mux);
        cv.wait(lock, [&]() { return currentWeight + peso_cliente <= weightLimit; });

        currentWeight += peso_cliente;
        cout << "Cliente " << id << " entrou no barco. Peso atual: " << currentWeight << ". Rodada " << i+1 << endl;

        lock.unlock();

        this_thread::sleep_for(chrono::milliseconds(10));

        lock.lock();
        currentWeight -= peso_cliente;
        cout << "Cliente " << id << " saiu do barco. Peso atual: " << currentWeight << ". Rodada " << i+1 << endl;

        lock.unlock();
        cv.notify_all();

        unique_lock<mutex> barrier_lock(barrier_mux);
        customersRollCompleted++;
        
        if (customersRollCompleted == customersNumber) {
            customersRollCompleted = 0;
            barrier_cv.notify_all();
        } else {
            barrier_cv.wait(barrier_lock);
        }
    }
}

int main() {

    cout << "Digite o limite de peso do barco: ";
    cin >> weightLimit;


    cout << "Digite o número de clientes: ";
    cin >> customersNumber;

    cout << "Digite o número de rodadas: ";
    cin >> timesRoll;

    vector<thread> threads;
    vector<int> weights = vector<int>(customersNumber);

    for (int i = 0; i < customersNumber; ++i) {
        cout << "Digite o peso do cliente " << i + 1 << ": ";
        cin >> weights[i];
    }

    for (int i = 0; i < customersNumber; ++i) {
        threads.push_back(thread(customerConsume, weights[i], i + 1));
    }

    for (thread &t : threads) {
        t.join();
    }

    return 0;
}