using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;
using System.IO;
using System.Threading;
using System.Diagnostics;


namespace StartBZFS
{
    public class LogScaner
    {
        public StreamReader reader;

        List<string> lines = new List<string>();

        object nugget = new object();

        public string popLine()
        {
            lock (nugget)
            {
                if (lines.Count > 0)
                {
                    string line = lines[0];
                    lines.RemoveAt(0);
                    return line;
                }
            }
            return string.Empty;
        }

        public LogScaner(StreamReader r)
        {
            reader = r;
        }

        public void read()
        {
            string line;
            line = reader.ReadLine();
            while ((line = reader.ReadLine()) != null)
            {
                lock (nugget)
                {
                    lines.Add(line);
                }
            }
        }
    }

    public class ServerLoger
    {
        public Process serverProc = new Process();

        ServerConfig config;

        ServerLogWindow logWindow;

        public ServerLoger(ServerConfig cfg)
        {
            config = cfg;
        }

        public void run(string command, string args, string tempLogiFile)
        {
            logWindow = new ServerLogWindow();
            logWindow.Show();

            serverProc.StartInfo.Arguments = args;// +redirectCommand(tempLogiFile);
            serverProc.StartInfo.FileName = command;
            if (!Prefrences.IsOSX())
                serverProc.StartInfo.WorkingDirectory = Path.GetDirectoryName(command);
        //    else
          //      serverProc.StartInfo.WorkingDirectory = Path.GetDirectoryName(command);

            serverProc.StartInfo.RedirectStandardError = true;
            serverProc.StartInfo.RedirectStandardOutput = true;
            serverProc.StartInfo.UseShellExecute = false;
            serverProc.StartInfo.CreateNoWindow = true;
            serverProc.Start();

            LogScaner logWriterStd = new LogScaner(serverProc.StandardOutput);
            LogScaner logWriterErr = new LogScaner(serverProc.StandardError);

            logWindow.addText("Server Started");

            Thread stdOutThread = new Thread(new ThreadStart(logWriterStd.read));
            Thread stdErrThread = new Thread(new ThreadStart(logWriterErr.read));

            stdOutThread.Start();
            stdErrThread.Start();

            while (!serverProc.HasExited)
            {
                if (logWindow.stop)
                    serverProc.Kill();
                else
                    Thread.Sleep(100);

                string line = logWriterStd.popLine();
                if (line != string.Empty)
                    logWindow.addText(line);

                line = logWriterErr.popLine();
                if (line != string.Empty)
                    logWindow.addText("Error: " + line);

                // just so we can do this single threaded for debugin
                Application.DoEvents();
            }
            stdOutThread.Abort();
            stdErrThread.Abort();
            //  logWindow.addText("Server Died man.. sorry);

            logWindow.Dispose();
            config.ServerTerminate(this);
        }
    }
}
