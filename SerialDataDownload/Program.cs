using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace SerialDataDownload
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            SelectPortForm startForm = new SelectPortForm();
            startForm.Show();
            Application.Run();
        }
    }
}
