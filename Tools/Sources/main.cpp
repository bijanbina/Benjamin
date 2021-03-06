#include <QGuiApplication>
#include "bt_chapar.h"
#include "bt_state.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    BtState st;
    if( argc>1 )
    {
        QString in_state = argv[1];
        if( in_state == "e" )
        {
            st.state = BT_ENN_MODE;
        }
        else if( in_state == "t" )
        {
            st.state = BT_TEST_MODE;
        }
    }

    BtChapar chaper(&st);

    return app.exec();
}
