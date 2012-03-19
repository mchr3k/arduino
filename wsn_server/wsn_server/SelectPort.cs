using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace wsn_server
{
    public partial class SelectPort : Form
    {
        private bool exit = true;

        public SelectPort()
        {
            InitializeComponent();
        }

        private void SelectPortForm_Load(object sender, EventArgs e)
        {
            string[] names = SerialPort.GetPortNames();
            Array.Sort(names);
            PortsDropDown.Items.Clear();
            foreach (String name in names)
            {
                PortsDropDown.Items.Add(name);
            }
            if (PortsDropDown.Items.Count >= 3)
            {
                PortsDropDown.SelectedIndex = 2;
            }
            else
            {
                PortsDropDown.SelectedIndex = 0;
            }            
        }

        private void SelectPortButton_Click(object sender, EventArgs e)
        {
            exit = false;

            Main conn = new Main(PortsDropDown.SelectedItem.ToString());            
            this.Close();
            conn.Show();
        }

        private void SelectPortForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (exit) Application.Exit();
        }

        private void NoneButton_Click(object sender, EventArgs e)
        {
            exit = false;

            Main conn = new Main();
            this.Close();
            conn.Show();
        }
    }
}
