namespace wsn_server
{
    partial class Main
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
            this.TableLayout = new System.Windows.Forms.TableLayoutPanel();
            this.SplitContainer = new System.Windows.Forms.SplitContainer();
            this.CommandTable = new System.Windows.Forms.TableLayoutPanel();
            this.TextOutput = new System.Windows.Forms.TextBox();
            this.CommandPanel = new System.Windows.Forms.Panel();
            this.CommandTextBox = new System.Windows.Forms.TextBox();
            this.SendButton = new System.Windows.Forms.Button();
            this.Graph = new ZedGraph.ZedGraphControl();
            this.ButtonPanel = new System.Windows.Forms.Panel();
            this.LoadButton = new System.Windows.Forms.Button();
            this.ClearButton = new System.Windows.Forms.Button();
            this.SaveButton = new System.Windows.Forms.Button();
            this.DownloadButton = new System.Windows.Forms.Button();
            this.TableLayout.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.SplitContainer)).BeginInit();
            this.SplitContainer.Panel1.SuspendLayout();
            this.SplitContainer.Panel2.SuspendLayout();
            this.SplitContainer.SuspendLayout();
            this.CommandTable.SuspendLayout();
            this.CommandPanel.SuspendLayout();
            this.ButtonPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // TableLayout
            // 
            this.TableLayout.ColumnCount = 1;
            this.TableLayout.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 150F));
            this.TableLayout.Controls.Add(this.SplitContainer, 0, 1);
            this.TableLayout.Controls.Add(this.ButtonPanel, 0, 0);
            this.TableLayout.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TableLayout.Location = new System.Drawing.Point(0, 0);
            this.TableLayout.Name = "TableLayout";
            this.TableLayout.RowCount = 2;
            this.TableLayout.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 30F));
            this.TableLayout.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.TableLayout.Size = new System.Drawing.Size(486, 431);
            this.TableLayout.TabIndex = 2;
            // 
            // SplitContainer
            // 
            this.SplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.SplitContainer.Location = new System.Drawing.Point(3, 33);
            this.SplitContainer.Name = "SplitContainer";
            // 
            // SplitContainer.Panel1
            // 
            this.SplitContainer.Panel1.Controls.Add(this.CommandTable);
            // 
            // SplitContainer.Panel2
            // 
            this.SplitContainer.Panel2.Controls.Add(this.Graph);
            this.SplitContainer.Size = new System.Drawing.Size(480, 395);
            this.SplitContainer.SplitterDistance = 100;
            this.SplitContainer.TabIndex = 5;
            // 
            // CommandTable
            // 
            this.CommandTable.AutoSize = true;
            this.CommandTable.ColumnCount = 1;
            this.CommandTable.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.CommandTable.Controls.Add(this.TextOutput, 0, 0);
            this.CommandTable.Controls.Add(this.CommandPanel, 0, 1);
            this.CommandTable.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CommandTable.Location = new System.Drawing.Point(0, 0);
            this.CommandTable.Name = "CommandTable";
            this.CommandTable.RowCount = 2;
            this.CommandTable.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.CommandTable.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.CommandTable.Size = new System.Drawing.Size(100, 395);
            this.CommandTable.TabIndex = 2;
            // 
            // TextOutput
            // 
            this.TextOutput.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TextOutput.Location = new System.Drawing.Point(3, 3);
            this.TextOutput.Multiline = true;
            this.TextOutput.Name = "TextOutput";
            this.TextOutput.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.TextOutput.Size = new System.Drawing.Size(94, 363);
            this.TextOutput.TabIndex = 2;
            this.TextOutput.WordWrap = false;
            // 
            // CommandPanel
            // 
            this.CommandPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.CommandPanel.Controls.Add(this.CommandTextBox);
            this.CommandPanel.Controls.Add(this.SendButton);
            this.CommandPanel.Location = new System.Drawing.Point(3, 372);
            this.CommandPanel.Name = "CommandPanel";
            this.CommandPanel.Size = new System.Drawing.Size(94, 20);
            this.CommandPanel.TabIndex = 3;
            // 
            // CommandTextBox
            // 
            this.CommandTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.CommandTextBox.Location = new System.Drawing.Point(0, 0);
            this.CommandTextBox.Name = "CommandTextBox";
            this.CommandTextBox.Size = new System.Drawing.Size(54, 20);
            this.CommandTextBox.TabIndex = 1;
            this.CommandTextBox.KeyUp += new System.Windows.Forms.KeyEventHandler(this.CommandTextBox_KeyUp);
            // 
            // SendButton
            // 
            this.SendButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.SendButton.Location = new System.Drawing.Point(54, 0);
            this.SendButton.Name = "SendButton";
            this.SendButton.Size = new System.Drawing.Size(40, 20);
            this.SendButton.TabIndex = 0;
            this.SendButton.Text = "Send";
            this.SendButton.UseVisualStyleBackColor = true;
            this.SendButton.Click += new System.EventHandler(this.SendButton_Click);
            // 
            // Graph
            // 
            this.Graph.Dock = System.Windows.Forms.DockStyle.Fill;
            this.Graph.Location = new System.Drawing.Point(0, 0);
            this.Graph.Name = "Graph";
            this.Graph.ScrollGrace = 0D;
            this.Graph.ScrollMaxX = 0D;
            this.Graph.ScrollMaxY = 0D;
            this.Graph.ScrollMaxY2 = 0D;
            this.Graph.ScrollMinX = 0D;
            this.Graph.ScrollMinY = 0D;
            this.Graph.ScrollMinY2 = 0D;
            this.Graph.Size = new System.Drawing.Size(376, 395);
            this.Graph.TabIndex = 4;
            // 
            // ButtonPanel
            // 
            this.ButtonPanel.Controls.Add(this.LoadButton);
            this.ButtonPanel.Controls.Add(this.ClearButton);
            this.ButtonPanel.Controls.Add(this.SaveButton);
            this.ButtonPanel.Controls.Add(this.DownloadButton);
            this.ButtonPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ButtonPanel.Location = new System.Drawing.Point(3, 3);
            this.ButtonPanel.Name = "ButtonPanel";
            this.ButtonPanel.Size = new System.Drawing.Size(480, 24);
            this.ButtonPanel.TabIndex = 2;
            // 
            // LoadButton
            // 
            this.LoadButton.Dock = System.Windows.Forms.DockStyle.Left;
            this.LoadButton.Location = new System.Drawing.Point(150, 0);
            this.LoadButton.Name = "LoadButton";
            this.LoadButton.Size = new System.Drawing.Size(75, 24);
            this.LoadButton.TabIndex = 5;
            this.LoadButton.Text = "Load";
            this.LoadButton.UseVisualStyleBackColor = true;
            this.LoadButton.Click += new System.EventHandler(this.LoadButton_Click);
            // 
            // ClearButton
            // 
            this.ClearButton.Dock = System.Windows.Forms.DockStyle.Right;
            this.ClearButton.Location = new System.Drawing.Point(405, 0);
            this.ClearButton.Name = "ClearButton";
            this.ClearButton.Size = new System.Drawing.Size(75, 24);
            this.ClearButton.TabIndex = 4;
            this.ClearButton.Text = "Clear";
            this.ClearButton.UseVisualStyleBackColor = true;
            this.ClearButton.Click += new System.EventHandler(this.ClearButton_Click);
            // 
            // SaveButton
            // 
            this.SaveButton.Dock = System.Windows.Forms.DockStyle.Left;
            this.SaveButton.Location = new System.Drawing.Point(75, 0);
            this.SaveButton.Name = "SaveButton";
            this.SaveButton.Size = new System.Drawing.Size(75, 24);
            this.SaveButton.TabIndex = 3;
            this.SaveButton.Text = "Save";
            this.SaveButton.UseVisualStyleBackColor = true;
            this.SaveButton.Click += new System.EventHandler(this.SaveButton_Click);
            // 
            // DownloadButton
            // 
            this.DownloadButton.Dock = System.Windows.Forms.DockStyle.Left;
            this.DownloadButton.Enabled = false;
            this.DownloadButton.Location = new System.Drawing.Point(0, 0);
            this.DownloadButton.Name = "DownloadButton";
            this.DownloadButton.Size = new System.Drawing.Size(75, 24);
            this.DownloadButton.TabIndex = 2;
            this.DownloadButton.Text = "Download";
            this.DownloadButton.UseVisualStyleBackColor = true;
            this.DownloadButton.Click += new System.EventHandler(this.DownloadButton_Click);
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(486, 431);
            this.Controls.Add(this.TableLayout);
            this.DoubleBuffered = true;
            this.Name = "Main";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Serial Temperature Data Download";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.SerialConnection_FormClosed);
            this.Load += new System.EventHandler(this.SerialConnection_Load);
            this.TableLayout.ResumeLayout(false);
            this.SplitContainer.Panel1.ResumeLayout(false);
            this.SplitContainer.Panel1.PerformLayout();
            this.SplitContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.SplitContainer)).EndInit();
            this.SplitContainer.ResumeLayout(false);
            this.CommandTable.ResumeLayout(false);
            this.CommandTable.PerformLayout();
            this.CommandPanel.ResumeLayout(false);
            this.CommandPanel.PerformLayout();
            this.ButtonPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel TableLayout;
        private System.Windows.Forms.Panel ButtonPanel;
        private System.Windows.Forms.Button SaveButton;
        private System.Windows.Forms.Button DownloadButton;
        private System.Windows.Forms.Button ClearButton;
        private System.Windows.Forms.SplitContainer SplitContainer;
        private ZedGraph.ZedGraphControl Graph;
        private System.Windows.Forms.Button LoadButton;
        private System.Windows.Forms.TableLayoutPanel CommandTable;
        private System.Windows.Forms.TextBox TextOutput;
        private System.Windows.Forms.Panel CommandPanel;
        private System.Windows.Forms.TextBox CommandTextBox;
        private System.Windows.Forms.Button SendButton;
    }
}