#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include <sstream>

using namespace std;

void send_recv_test();
void broadcast_test();
void gather_test();
void gatherv_test();

int main ( int argc, char * argv[] )
{
    /* MPI initialisieren */
    MPI_Init ( &argc, &argv );

    int my_rank, world_size;
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &world_size );

    if (world_size < 2)
    {
        cout << "Error: use at least 2 processes." << endl;
        exit(1);
    }

    if ( argc < 2 )
    {
        if ( my_rank == 0 )
        {
            cout << "Usage: " << argv[0] << " TEST" << endl;
            cout << "TEST:  send_recv" << endl;
            cout << "       broadcast" << endl;
            cout << "       gather" << endl;
            cout << "       gatherv" << endl;
        }

        exit ( 1 );
    }
    else
    {
        string argv1 = argv[1];

        if ( argv1.compare ( "send_recv" ) == 0 )
        {
            send_recv_test();
        }
        else if ( argv1.compare ( "broadcast" ) == 0 )
        {
            broadcast_test();
        }
        else if ( argv1.compare ( "gather" ) == 0 )
        {
            gather_test();
        }
        else if ( argv1.compare ( "gatherv" ) == 0 )
        {
            gatherv_test();
        }
        else
        {
            if ( my_rank == 0 )
            {
                cout << "not match!" << endl;
            }

            exit ( 1 );
        }
    }

    /* MPI beenden */
    MPI_Finalize();

    return 0;
}

void send_recv_test()
{
    int my_rank, world_size;
    MPI_Status status;

    stringstream s; /* fuer schoenere Ausgaben */

    /* Rang und Prozesszahl ermitteln */
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &world_size );

    /* Buffer */
    int send_data[10];
    int recv_data[10];

    for ( int i = 0; i < 10; i++ )
    {
        send_data[i] = my_rank;
    }

    int destination = ( my_rank + 1 ) % world_size;

    /* Fallunterscheidung um Deadlock zu vermeiden */
    if ( my_rank % 2 == 0 )
    {
        /* Sende an Nachfolger */
        MPI_Send ( send_data, 10, MPI_INT, destination, 0, MPI_COMM_WORLD );

        s << "Process " << my_rank << ": sent data to process " << destination << endl;
        cout << s.str();
        s.str ( "" );

        /* Empfange von Vorgaenger */
        MPI_Recv ( recv_data, 10, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

        s << "Process " << my_rank << ": received data from process " << status.MPI_SOURCE << endl;
        cout << s.str();
        s.str ( "" );
    }
    else
    {
        /* Empfange von Vorgaenger */
        MPI_Recv ( recv_data, 10, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );

        s << "Process " << my_rank << ": received data from process " << status.MPI_SOURCE << endl;
        cout << s.str();
        s.str ( "" );

        /* Sende an Nachfolger */
        MPI_Send ( send_data, 10, MPI_INT, destination, 0, MPI_COMM_WORLD );

        s << "Process " << my_rank << ": sent data to process " << destination << endl;
        cout << s.str();
        s.str ( "" );
    }
}

void broadcast_test()
{
    // Buffer
    int data[10];

    int my_rank;

    stringstream s; // fuer schoenere Ausgaben

    // Rang ermitteln
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );

    if ( my_rank == 0 )
    {
        // Wurzelprozess hat Daten
        for ( int i = 0; i < 10; i++ )
        {
            data[i] = 42;
        }
    }

    // Alle Prozesse nehmen an der Broadcast-Operation teil, dadurch erhalten sie Daten vom Wurzelprozess
    MPI_Bcast ( data, 10, MPI_INT, 0, MPI_COMM_WORLD );

    s << "Process " << my_rank << " received data: ";

    for ( int i = 0; i < 10; i++ )
    {
        s << data[i] << " ";
    }

    s << endl;

    cout << s.str();
}

void gather_test()
{
    int my_rank, world_size;

    /* Rang und Prozesszahl ermitteln */
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &world_size );

    int * recv_buffer;

    if ( my_rank == 0 )
    {
        // Wurzelprozess erstellt Buffer
        recv_buffer = new int[world_size];
    }

    // Prozesse senden ihre ID an den Wurzelprozess
    MPI_Gather ( &my_rank, 1, MPI_INT, recv_buffer, 1, MPI_INT, 0, MPI_COMM_WORLD );

    if ( my_rank == 0 )
    {
        cout << "Root: received " << world_size << " elements: ";

        for ( int i = 0; i < world_size; i++ )
        {
            cout << recv_buffer[i] << " ";
        }

        cout << endl;
    }

    if ( my_rank == 0 )
    {
        delete [] recv_buffer;
    }
}

void gatherv_test()
{
    int my_rank, world_size;

    /* Rang und Prozesszahl ermitteln */
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &world_size );

    int * send_buffer, * recv_buffer;
    int * recv_counts, * displacements;

    // Jeder Prozess sendet my_rank + 1 Elemente, d.h. Prozess 0 sendet 1, Prozess 1 sendet 2, usw.
    int send_count = my_rank + 1;

    // Sendepuffer entsprechender Groesse anlegen ...
    send_buffer = new int[send_count];

    // ... und fuellen
    for ( int i = 0; i < send_count; i++ )
    {
        send_buffer[i] = my_rank;
    }

    if ( my_rank == 0 )
    {
        // Wurzelprozess legt Empfangspuffer an
        // Groesse: 1 + 2 + ... + world_size = world_size * (world_size+1) / 2
        recv_buffer = new int[world_size * ( world_size + 1 ) / 2];

        // Anzahl der Elemente, die von jedem Prozess empfangen werden
        recv_counts = new int[world_size];

        // Displacements festlegen
        displacements = new int[world_size];

        displacements[0] = 0;
        recv_counts[0] = 1;

        for ( int i = 1; i < world_size; i++ )
        {
            recv_counts[i] = i + 1;
            displacements[i] = displacements[i - 1] + i;
        }
    }

    // Prozesse senden Daten an den Wurzelprozess
    MPI_Gatherv ( send_buffer, send_count, MPI_INT, recv_buffer, recv_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD );

    if ( my_rank == 0 )
    {
        cout << "Root: received " << world_size * ( world_size + 1 ) / 2 << " elements: ";

        for ( int i = 0; i < world_size * ( world_size + 1 ) / 2; i++ )
        {
            cout << recv_buffer[i] << " ";
        }

        cout << endl;
    }

    delete [] send_buffer;

    if ( my_rank == 0 )
    {
        delete [] recv_buffer;
        delete [] recv_counts;
        delete [] displacements;
    }
}
