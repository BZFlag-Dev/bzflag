namespace StartBZFS
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fIleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pathsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.Status = new System.Windows.Forms.ToolStripStatusLabel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.World = new System.Windows.Forms.GroupBox();
            this.SpawnOnBoxes = new System.Windows.Forms.CheckBox();
            this.Teleporters = new System.Windows.Forms.CheckBox();
            this.WorldsList = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.LogLevel = new System.Windows.Forms.ComboBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.Antidote = new System.Windows.Forms.CheckBox();
            this.FlagsOnBuildings = new System.Windows.Forms.CheckBox();
            this.Ricochet = new System.Windows.Forms.CheckBox();
            this.ShakeTime = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.ShakeWins = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.BadFlags = new System.Windows.Forms.CheckBox();
            this.GoodFlags = new System.Windows.Forms.CheckBox();
            this.NumShots = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.Jumping = new System.Windows.Forms.CheckBox();
            this.RabbitMode = new System.Windows.Forms.RadioButton();
            this.CTFMode = new System.Windows.Forms.RadioButton();
            this.OFFAMode = new System.Windows.Forms.RadioButton();
            this.FFAMode = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ServerAddress = new System.Windows.Forms.TextBox();
            this.ServerTest = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.PublicServer = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.ServerPort = new System.Windows.Forms.TextBox();
            this.Start = new System.Windows.Forms.Button();
            this.RunInBackground = new System.Windows.Forms.CheckBox();
            this.menuStrip1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.World.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fIleToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(552, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fIleToolStripMenuItem
            // 
            this.fIleToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.pathsToolStripMenuItem});
            this.fIleToolStripMenuItem.Name = "fIleToolStripMenuItem";
            this.fIleToolStripMenuItem.Size = new System.Drawing.Size(61, 20);
            this.fIleToolStripMenuItem.Text = "Options";
            // 
            // pathsToolStripMenuItem
            // 
            this.pathsToolStripMenuItem.Name = "pathsToolStripMenuItem";
            this.pathsToolStripMenuItem.Size = new System.Drawing.Size(112, 22);
            this.pathsToolStripMenuItem.Text = "Paths...";
            this.pathsToolStripMenuItem.Click += new System.EventHandler(this.pathsToolStripMenuItem_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.statusStrip1.Dock = System.Windows.Forms.DockStyle.None;
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.Status});
            this.statusStrip1.Location = new System.Drawing.Point(0, 329);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(59, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // Status
            // 
            this.Status.Name = "Status";
            this.Status.Size = new System.Drawing.Size(42, 17);
            this.Status.Text = "Status:";
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.Controls.Add(this.World);
            this.panel1.Controls.Add(this.LogLevel);
            this.panel1.Controls.Add(this.groupBox3);
            this.panel1.Controls.Add(this.label4);
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Location = new System.Drawing.Point(12, 27);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(528, 263);
            this.panel1.TabIndex = 2;
            // 
            // World
            // 
            this.World.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.World.Controls.Add(this.SpawnOnBoxes);
            this.World.Controls.Add(this.Teleporters);
            this.World.Controls.Add(this.WorldsList);
            this.World.Controls.Add(this.label5);
            this.World.Location = new System.Drawing.Point(3, 153);
            this.World.Name = "World";
            this.World.Size = new System.Drawing.Size(203, 100);
            this.World.TabIndex = 7;
            this.World.TabStop = false;
            this.World.Text = "World";
            // 
            // SpawnOnBoxes
            // 
            this.SpawnOnBoxes.AutoSize = true;
            this.SpawnOnBoxes.Location = new System.Drawing.Point(10, 67);
            this.SpawnOnBoxes.Name = "SpawnOnBoxes";
            this.SpawnOnBoxes.Size = new System.Drawing.Size(108, 17);
            this.SpawnOnBoxes.TabIndex = 3;
            this.SpawnOnBoxes.Text = "Spawn On Boxes";
            this.SpawnOnBoxes.UseVisualStyleBackColor = true;
            // 
            // Teleporters
            // 
            this.Teleporters.AutoSize = true;
            this.Teleporters.Location = new System.Drawing.Point(10, 44);
            this.Teleporters.Name = "Teleporters";
            this.Teleporters.Size = new System.Drawing.Size(79, 17);
            this.Teleporters.TabIndex = 2;
            this.Teleporters.Text = "Teleporters";
            this.Teleporters.UseVisualStyleBackColor = true;
            // 
            // WorldsList
            // 
            this.WorldsList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.WorldsList.FormattingEnabled = true;
            this.WorldsList.Location = new System.Drawing.Point(48, 17);
            this.WorldsList.Name = "WorldsList";
            this.WorldsList.Size = new System.Drawing.Size(139, 21);
            this.WorldsList.TabIndex = 1;
            this.WorldsList.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(7, 20);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(35, 13);
            this.label5.TabIndex = 0;
            this.label5.Text = "World";
            // 
            // LogLevel
            // 
            this.LogLevel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.LogLevel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.LogLevel.FormattingEnabled = true;
            this.LogLevel.Items.AddRange(new object[] {
            "None",
            "1",
            "2",
            "3",
            "4"});
            this.LogLevel.Location = new System.Drawing.Point(481, 237);
            this.LogLevel.Name = "LogLevel";
            this.LogLevel.Size = new System.Drawing.Size(44, 21);
            this.LogLevel.TabIndex = 3;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.Antidote);
            this.groupBox3.Controls.Add(this.FlagsOnBuildings);
            this.groupBox3.Controls.Add(this.Ricochet);
            this.groupBox3.Controls.Add(this.ShakeTime);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.ShakeWins);
            this.groupBox3.Controls.Add(this.label6);
            this.groupBox3.Controls.Add(this.BadFlags);
            this.groupBox3.Controls.Add(this.GoodFlags);
            this.groupBox3.Controls.Add(this.NumShots);
            this.groupBox3.Controls.Add(this.label3);
            this.groupBox3.Location = new System.Drawing.Point(96, 4);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(228, 116);
            this.groupBox3.TabIndex = 6;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Shots and Flags";
            // 
            // Antidote
            // 
            this.Antidote.AutoSize = true;
            this.Antidote.Location = new System.Drawing.Point(126, 91);
            this.Antidote.Name = "Antidote";
            this.Antidote.Size = new System.Drawing.Size(70, 17);
            this.Antidote.TabIndex = 12;
            this.Antidote.Text = "Antidotes";
            this.Antidote.UseVisualStyleBackColor = true;
            // 
            // FlagsOnBuildings
            // 
            this.FlagsOnBuildings.AutoSize = true;
            this.FlagsOnBuildings.Location = new System.Drawing.Point(9, 91);
            this.FlagsOnBuildings.Name = "FlagsOnBuildings";
            this.FlagsOnBuildings.Size = new System.Drawing.Size(111, 17);
            this.FlagsOnBuildings.TabIndex = 11;
            this.FlagsOnBuildings.Text = "Flags on Buildings";
            this.FlagsOnBuildings.UseVisualStyleBackColor = true;
            // 
            // Ricochet
            // 
            this.Ricochet.AutoSize = true;
            this.Ricochet.Location = new System.Drawing.Point(98, 19);
            this.Ricochet.Name = "Ricochet";
            this.Ricochet.Size = new System.Drawing.Size(69, 17);
            this.Ricochet.TabIndex = 10;
            this.Ricochet.Text = "Ricochet";
            this.Ricochet.UseVisualStyleBackColor = true;
            // 
            // ShakeTime
            // 
            this.ShakeTime.Location = new System.Drawing.Point(166, 66);
            this.ShakeTime.Name = "ShakeTime";
            this.ShakeTime.Size = new System.Drawing.Size(49, 20);
            this.ShakeTime.TabIndex = 9;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(95, 69);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(64, 13);
            this.label7.TabIndex = 8;
            this.label7.Text = "Shake Time";
            // 
            // ShakeWins
            // 
            this.ShakeWins.Location = new System.Drawing.Point(166, 43);
            this.ShakeWins.Name = "ShakeWins";
            this.ShakeWins.Size = new System.Drawing.Size(49, 20);
            this.ShakeWins.TabIndex = 7;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(95, 46);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(65, 13);
            this.label6.TabIndex = 6;
            this.label6.Text = "Shake Wins";
            // 
            // BadFlags
            // 
            this.BadFlags.AutoSize = true;
            this.BadFlags.Location = new System.Drawing.Point(9, 68);
            this.BadFlags.Name = "BadFlags";
            this.BadFlags.Size = new System.Drawing.Size(73, 17);
            this.BadFlags.TabIndex = 3;
            this.BadFlags.Text = "Bad Flags";
            this.BadFlags.UseVisualStyleBackColor = true;
            this.BadFlags.CheckedChanged += new System.EventHandler(this.BadFlags_CheckedChanged);
            // 
            // GoodFlags
            // 
            this.GoodFlags.AutoSize = true;
            this.GoodFlags.Location = new System.Drawing.Point(9, 45);
            this.GoodFlags.Name = "GoodFlags";
            this.GoodFlags.Size = new System.Drawing.Size(80, 17);
            this.GoodFlags.TabIndex = 2;
            this.GoodFlags.Text = "Good Flags";
            this.GoodFlags.UseVisualStyleBackColor = true;
            // 
            // NumShots
            // 
            this.NumShots.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.NumShots.FormattingEnabled = true;
            this.NumShots.Items.AddRange(new object[] {
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "10",
            "11",
            "12",
            "13",
            "14",
            "15",
            "16",
            "17",
            "19",
            "20"});
            this.NumShots.Location = new System.Drawing.Point(46, 18);
            this.NumShots.Name = "NumShots";
            this.NumShots.Size = new System.Drawing.Size(44, 21);
            this.NumShots.TabIndex = 1;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 18);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(34, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Shots";
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(401, 240);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(74, 13);
            this.label4.TabIndex = 2;
            this.label4.Text = "Logging Level";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.Jumping);
            this.groupBox2.Controls.Add(this.RabbitMode);
            this.groupBox2.Controls.Add(this.CTFMode);
            this.groupBox2.Controls.Add(this.OFFAMode);
            this.groupBox2.Controls.Add(this.FFAMode);
            this.groupBox2.Location = new System.Drawing.Point(3, 3);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(91, 142);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Game Mode";
            // 
            // Jumping
            // 
            this.Jumping.AutoSize = true;
            this.Jumping.Location = new System.Drawing.Point(7, 113);
            this.Jumping.Name = "Jumping";
            this.Jumping.Size = new System.Drawing.Size(65, 17);
            this.Jumping.TabIndex = 4;
            this.Jumping.Text = "Jumping";
            this.Jumping.UseVisualStyleBackColor = true;
            // 
            // RabbitMode
            // 
            this.RabbitMode.AutoSize = true;
            this.RabbitMode.Location = new System.Drawing.Point(6, 89);
            this.RabbitMode.Name = "RabbitMode";
            this.RabbitMode.Size = new System.Drawing.Size(56, 17);
            this.RabbitMode.TabIndex = 3;
            this.RabbitMode.TabStop = true;
            this.RabbitMode.Text = "Rabbit";
            this.RabbitMode.UseVisualStyleBackColor = true;
            // 
            // CTFMode
            // 
            this.CTFMode.AutoSize = true;
            this.CTFMode.Location = new System.Drawing.Point(6, 66);
            this.CTFMode.Name = "CTFMode";
            this.CTFMode.Size = new System.Drawing.Size(45, 17);
            this.CTFMode.TabIndex = 2;
            this.CTFMode.TabStop = true;
            this.CTFMode.Text = "CTF";
            this.CTFMode.UseVisualStyleBackColor = true;
            this.CTFMode.CheckedChanged += new System.EventHandler(this.CTFMode_CheckedChanged);
            // 
            // OFFAMode
            // 
            this.OFFAMode.AutoSize = true;
            this.OFFAMode.Location = new System.Drawing.Point(6, 43);
            this.OFFAMode.Name = "OFFAMode";
            this.OFFAMode.Size = new System.Drawing.Size(73, 17);
            this.OFFAMode.TabIndex = 1;
            this.OFFAMode.TabStop = true;
            this.OFFAMode.Text = "Open FFA";
            this.OFFAMode.UseVisualStyleBackColor = true;
            // 
            // FFAMode
            // 
            this.FFAMode.AutoSize = true;
            this.FFAMode.Location = new System.Drawing.Point(6, 20);
            this.FFAMode.Name = "FFAMode";
            this.FFAMode.Size = new System.Drawing.Size(44, 17);
            this.FFAMode.TabIndex = 0;
            this.FFAMode.TabStop = true;
            this.FFAMode.Text = "FFA";
            this.FFAMode.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.ServerAddress);
            this.groupBox1.Controls.Add(this.ServerTest);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.PublicServer);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.ServerPort);
            this.groupBox1.Location = new System.Drawing.Point(326, 4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(200, 128);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Address";
            // 
            // ServerAddress
            // 
            this.ServerAddress.Location = new System.Drawing.Point(59, 44);
            this.ServerAddress.Name = "ServerAddress";
            this.ServerAddress.Size = new System.Drawing.Size(135, 20);
            this.ServerAddress.TabIndex = 5;
            // 
            // ServerTest
            // 
            this.ServerTest.Enabled = false;
            this.ServerTest.Location = new System.Drawing.Point(119, 99);
            this.ServerTest.Name = "ServerTest";
            this.ServerTest.Size = new System.Drawing.Size(75, 23);
            this.ServerTest.TabIndex = 0;
            this.ServerTest.Text = "Test";
            this.ServerTest.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(45, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Address";
            // 
            // PublicServer
            // 
            this.PublicServer.AutoSize = true;
            this.PublicServer.Location = new System.Drawing.Point(7, 20);
            this.PublicServer.Name = "PublicServer";
            this.PublicServer.Size = new System.Drawing.Size(55, 17);
            this.PublicServer.TabIndex = 3;
            this.PublicServer.Text = "Public";
            this.PublicServer.UseVisualStyleBackColor = true;
            this.PublicServer.CheckedChanged += new System.EventHandler(this.PublicServer_CheckedChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(29, 71);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(26, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Port";
            // 
            // ServerPort
            // 
            this.ServerPort.Location = new System.Drawing.Point(61, 68);
            this.ServerPort.Name = "ServerPort";
            this.ServerPort.Size = new System.Drawing.Size(46, 20);
            this.ServerPort.TabIndex = 2;
            this.ServerPort.Text = "5154";
            // 
            // Start
            // 
            this.Start.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.Start.Location = new System.Drawing.Point(465, 296);
            this.Start.Name = "Start";
            this.Start.Size = new System.Drawing.Size(75, 23);
            this.Start.TabIndex = 3;
            this.Start.Text = "Start";
            this.Start.UseVisualStyleBackColor = true;
            this.Start.Click += new System.EventHandler(this.Start_Click);
            // 
            // RunInBackground
            // 
            this.RunInBackground.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.RunInBackground.AutoSize = true;
            this.RunInBackground.Checked = true;
            this.RunInBackground.CheckState = System.Windows.Forms.CheckState.Checked;
            this.RunInBackground.Enabled = false;
            this.RunInBackground.Location = new System.Drawing.Point(337, 302);
            this.RunInBackground.Name = "RunInBackground";
            this.RunInBackground.Size = new System.Drawing.Size(117, 17);
            this.RunInBackground.TabIndex = 4;
            this.RunInBackground.Text = "Run in background";
            this.RunInBackground.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(552, 351);
            this.Controls.Add(this.RunInBackground);
            this.Controls.Add(this.Start);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "StartBZFS";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.World.ResumeLayout(false);
            this.World.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fIleToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pathsToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel Status;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TextBox ServerPort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button ServerTest;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox ServerAddress;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox PublicServer;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton RabbitMode;
        private System.Windows.Forms.RadioButton CTFMode;
        private System.Windows.Forms.RadioButton OFFAMode;
        private System.Windows.Forms.RadioButton FFAMode;
        private System.Windows.Forms.Button Start;
        private System.Windows.Forms.CheckBox RunInBackground;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.ComboBox NumShots;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox LogLevel;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox BadFlags;
        private System.Windows.Forms.CheckBox GoodFlags;
        private System.Windows.Forms.TextBox ShakeTime;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox ShakeWins;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.CheckBox FlagsOnBuildings;
        private System.Windows.Forms.CheckBox Ricochet;
        private System.Windows.Forms.CheckBox Jumping;
        private System.Windows.Forms.CheckBox Antidote;
        private System.Windows.Forms.GroupBox World;
        private System.Windows.Forms.CheckBox SpawnOnBoxes;
        private System.Windows.Forms.CheckBox Teleporters;
        private System.Windows.Forms.ComboBox WorldsList;
        private System.Windows.Forms.Label label5;
    }
}

