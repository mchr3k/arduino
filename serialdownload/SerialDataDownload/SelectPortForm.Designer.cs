namespace SerialDataDownload
{
    partial class SelectPortForm
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
            this.PortsDropDown = new System.Windows.Forms.ComboBox();
            this.SelectPortButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // PortsDropDown
            // 
            this.PortsDropDown.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.PortsDropDown.FormattingEnabled = true;
            this.PortsDropDown.Location = new System.Drawing.Point(12, 12);
            this.PortsDropDown.Name = "PortsDropDown";
            this.PortsDropDown.Size = new System.Drawing.Size(218, 21);
            this.PortsDropDown.TabIndex = 1;
            // 
            // SelectPortButton
            // 
            this.SelectPortButton.Location = new System.Drawing.Point(236, 12);
            this.SelectPortButton.Name = "SelectPortButton";
            this.SelectPortButton.Size = new System.Drawing.Size(75, 23);
            this.SelectPortButton.TabIndex = 2;
            this.SelectPortButton.Text = "OK";
            this.SelectPortButton.UseVisualStyleBackColor = true;
            this.SelectPortButton.Click += new System.EventHandler(this.SelectPortButton_Click);
            // 
            // SelectPortForm
            // 
            this.AcceptButton = this.SelectPortButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(320, 44);
            this.Controls.Add(this.SelectPortButton);
            this.Controls.Add(this.PortsDropDown);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "SelectPortForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Select Port";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.SelectPortForm_FormClosed);
            this.Load += new System.EventHandler(this.SelectPortForm_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox PortsDropDown;
        private System.Windows.Forms.Button SelectPortButton;
    }
}

