using System.Windows;

namespace mustard
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private System.Diagnostics.Process mProcessEngine = null;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void onWindowsFormsHostLoaded(object sender, RoutedEventArgs e)
        {
            var processInfo = new System.Diagnostics.ProcessStartInfo();
            //processInfo.FileName = Environment.Is64BitProcess ? "..\\bin_x64\\horseradish.exe" : "..\\bin_x86\\horseradish.exe";
            processInfo.FileName = "..\\bin_x86\\horseradish.exe";
            processInfo.Arguments = string.Format("--editor=0x{0}", windowsFormHost.Handle.ToString("X"));

            try
            {
                mProcessEngine = System.Diagnostics.Process.Start(processInfo);
                mProcessEngine.Exited += onProcessEngine_Exited;
            }
            catch
            {
                mProcessEngine = null;
            }
            
            if (mProcessEngine == null || mProcessEngine.HasExited)
            {
                mProcessEngine = null;

                MessageBox.Show("Unable to start engine process.\nUnable to continue.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return;
            }
        }

        private void onProcessEngine_Exited(object sender, System.EventArgs e)
        {
            mProcessEngine = null;

            MessageBox.Show("Lost connection with the engine.\nUnable to continue.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            Application.Current.Shutdown();
        }

        private void onExit(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (mProcessEngine != null)
            {
                try
                {
                    mProcessEngine.CloseMainWindow();
                    mProcessEngine.WaitForExit();
                }
                catch { }

                mProcessEngine = null;
            }
        }
    }
}
