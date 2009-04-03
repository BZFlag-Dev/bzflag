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
            this.groupBox2 = new System.Windows.Forms.GroupBox();
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
            this.Teams = new System.Windows.Forms.Button();
            this.Start = new System.Windows.Forms.Button();
            this.RunInBackground = new System.Windows.Forms.CheckBox();
            this.menuStrip1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.panel1.SuspendLayout();
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
            this.menuStrip1.Size = new System.Drawing.Size(422, 24);
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
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.Status});
            this.statusStrip1.Location = new System.Drawing.Point(0, 309);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(422, 22);
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
            this.panel1.Controls.Add(this.Teams);
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Location = new System.Drawing.Point(12, 27);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(398, 222);
            this.panel1.TabIndex = 2;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.RabbitMode);
            this.groupBox2.Controls.Add(this.CTFMode);
            this.groupBox2.Controls.Add(this.OFFAMode);
            this.groupBox2.Controls.Add(this.FFAMode);
            this.groupBox2.Location = new System.Drawing.Point(3, 3);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(91, 128);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Game Mode";
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
            this.groupBox1.Controls.Add(this.ServerAddress);
            this.groupBox1.Controls.Add(this.ServerTest);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.PublicServer);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.ServerPort);
            this.groupBox1.Location = new System.Drawing.Point(195, 3);
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
            // Teams
            // 
            this.Teams.Location = new System.Drawing.Point(3, 196);
            this.Teams.Name = "Teams";
            this.Teams.Size = new System.Drawing.Size(75, 23);
            this.Teams.TabIndex = 5;
            this.Teams.Text = "Teams";
            this.Teams.UseVisualStyleBackColor = true;
            // 
            // Start
            // 
            this.Start.Location = new System.Drawing.Point(335, 268);
            this.Start.Name = "Start";
            this.Start.Size = new System.Drawing.Size(75, 23);
            this.Start.TabIndex = 3;
            this.Start.Text = "Start";
            this.Start.UseVisualStyleBackColor = true;
            this.Start.Click += new System.EventHandler(this.Start_Click);
            // 
            // RunInBackground
            // 
            this.RunInBackground.AutoSize = true;
            this.RunInBackground.Location = new System.Drawing.Point(207, 274);
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
            this.ClientSize = new System.Drawing.Size(422, 331);
            this.Controls.Add(this.RunInBackground);
            this.Controls.Add(this.Start);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "StartBZFS";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.panel1.ResumeLayout(false);
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
        private System.Windows.Forms.Button Teams;
        private System.Windows.Forms.Button Start;
        private System.Windows.Forms.CheckBox RunInBackground;
    }
}

