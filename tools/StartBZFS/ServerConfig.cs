using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StartBZFS
{
    public enum GameMode
    {
        FFA,
        OpenFFA,
        CTF,
        Rabbit
    }

    class ServerConfig
    {
        public GameMode mode = GameMode.FFA;
        public bool publicServer = false;

        public string serverAddress = "localhost";
        public int port = 5154;
    }
}
