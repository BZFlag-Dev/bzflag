using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace StartBZFS
{
    public class Prefrences
    {
        public static bool IsWinddows()
        {
            return Environment.OSVersion.Platform == PlatformID.Win32Windows || Environment.OSVersion.Platform == PlatformID.Win32NT;
        }
        public static bool IsUnix()
        {
            return Environment.OSVersion.Platform == PlatformID.Unix;
        }

        public static bool IsOSX()
        {
            return Environment.OSVersion.Platform != PlatformID.Unix && Environment.OSVersion.Platform != PlatformID.Win32Windows;
        }

        public static DirectoryInfo GetConfigDir()
        {
            if (IsUnix())
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Personal),".bzf"));
            else if (IsOSX())
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "BZFlag"));
            else// windows
                return new DirectoryInfo(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "My BZFlag Files"));
        }

        private static DirectoryInfo FindWindowsInstallDir()
        {
            // ok so this is lame, BUT, check the program files dirs for stuff on the root drive.
            // if we had a registrty key then we'd be cool, but it clears itself after we exit
            DirectoryInfo programFiles = new DirectoryInfo(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles));

            foreach( DirectoryInfo d in programFiles.GetDirectories("BZFlag*",SearchOption.TopDirectoryOnly))
            {
                return d;
            }

            DirectoryInfo programFiles32 = new DirectoryInfo(programFiles.FullName + " (x86)");
            if (programFiles32.Exists)
            {
                foreach (DirectoryInfo d in programFiles32.GetDirectories("BZFlag*", SearchOption.TopDirectoryOnly))
                {
                    return d;
                }
            }

            return null;
        }

        private static FileInfo FindWindowsClient()
        {
            DirectoryInfo installDir = FindWindowsInstallDir();
            if (installDir == null || !installDir.Exists)
                return null;

            return new FileInfo(Path.Combine(installDir.FullName, "bzflag.exe"));
        }


        private static FileInfo FindWindowsServer()
        {
            DirectoryInfo installDir = FindWindowsInstallDir();
            if (installDir == null || !installDir.Exists)
                return null;

            return new FileInfo(Path.Combine(installDir.FullName, "bzfs.exe"));
        }

        public static FileInfo FindClient(DirectoryInfo confDir)
        {
            // try some tricky stuff to find windows
            if (IsWinddows())
            {
                FileInfo windowsClient = FindWindowsClient();
                if (windowsClient != null && windowsClient.Exists)
                    return windowsClient;
            }
            else if (IsUnix())
            {
                // path stuff?
            }
            else if (IsOSX())
            {
                // checking the application dir?
            }

            // ok, it can't be found by just common sense searching, see if it's a new verson that wrote out it's dir files

            FileInfo client = new FileInfo("\\");

            FileInfo clientDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzflag.dir"));
            if (clientDirFile.Exists)
            {
                StreamReader sr = clientDirFile.OpenText();
                sr.ReadLine();
                client = new FileInfo(sr.ReadLine());
                sr.Close();
            }

            return client;
        }

        public static FileInfo FindServer (DirectoryInfo confDir)
        {
            // try some tricky stuff to find windows
            if (IsWinddows())
            {
                FileInfo windowsServer = FindWindowsServer();
                if (windowsServer != null && windowsServer.Exists)
                    return windowsServer;
            }
            else if (IsUnix())
            {
                // path stuff?
            }
            else if (IsOSX())
            {
                // checking the application dir?
            } 
            
            FileInfo server = new FileInfo("\\");

            FileInfo serverDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzfs.dir"));
            if (serverDirFile.Exists)
            {
                StreamReader sr = serverDirFile.OpenText();
                server = new FileInfo(sr.ReadLine());
                sr.Close();
            }

            if (!server.Exists)
            {
                // the server file was bogus, try the client path with "bzfs" on it

                FileInfo clientDirFile = new FileInfo(Path.Combine(confDir.FullName, "bzflag.dir"));
                if (clientDirFile.Exists)
                {
                    string exeFile, exePath;
                    StreamReader sr = clientDirFile.OpenText();
                    exePath = sr.ReadLine();
                    exeFile = sr.ReadLine();
                    sr.Close();

                    server = new FileInfo(Path.Combine(exePath, "bzfs"));
                    if (!server.Exists)
                    {
                        server = new FileInfo(Path.Combine(exePath, "bzfs.exe"));
                        if (!server.Exists)
                        {
                            // ok so we couldn't find it in the stored bzflag dir, so try the exe path
                            exePath = Path.GetDirectoryName(exeFile);
                            server = new FileInfo(Path.Combine(exePath, "bzfs"));
                            if (!server.Exists)
                                server = new FileInfo(Path.Combine(exePath, "bzfs.exe"));
                        }
                    }
                }
            }

            return server;
        }

        public static DirectoryInfo FindWorldDir ( DirectoryInfo confDir )
        {
            DirectoryInfo worldDir = new DirectoryInfo(Path.Combine(confDir.FullName, "worlds"));
            if (!worldDir.Exists)
            {
                worldDir.Create();
                worldDir = new DirectoryInfo(worldDir.FullName);
            }
            return worldDir;
        }

        public string ServerPath = string.Empty;
        public string ClientPath = string.Empty;
        public string WorldPath = string.Empty;

        public bool LocatePaths ( DirectoryInfo configDir )
        {
            return false;
        }
    }
}
