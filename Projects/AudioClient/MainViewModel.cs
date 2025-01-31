using NLog;

namespace AudioClient;

using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;

public class MainViewModel : INotifyPropertyChanged
{
    public AudioDeviceService AudioDeviceService { get; }

    public static Dispatcher? MyDispatcher { get; private set; }
    private AudioDeviceInfo? _device;

    public AudioDeviceInfo? Device
    {
        get => _device;
        set
        {
            // ReSharper disable once InvertIf
            if (_device != value)
            {
                _device = value;
                OnPropertyChanged(nameof(Device));
            }
        }
    }

    public string WindowTitle { get; }

    private readonly AcLogDelegate _logMeDelegate = LogMe;
    private readonly AcEventDelegate _onDeviceAttachDetach = OnDeviceAttachDetach;

    public MainViewModel()
    {
        MyDispatcher = Dispatcher.CurrentDispatcher;

        var logger = LogManager.GetCurrentClassLogger();
        var filter = "";
        var args = Environment.GetCommandLineArgs();
        if (args.Length > 1)
        {
            filter = args[1];
            logger.Info($"Command line parameter(s) detected. The first one is {(filter.Length == 0 ? "empty string" : filter)}.");
        }
        else
        {
            logger.Info("No command line parameters detected");
        }

        WindowTitle = "Audio Control, ";
        if (!string.IsNullOrEmpty(filter))
        {
            WindowTitle += $"filter substring: \"{filter}\"";
        }
        else
        {
            WindowTitle += "no filter. You can set filter via command line";
        }

        AudioDeviceService = new AudioDeviceService(_onDeviceAttachDetach, _logMeDelegate, filter);
        var audioDeviceInfo = AudioDeviceService.GetAudioDevice();
        Device = audioDeviceInfo.PnPUuid.Length != 0 ? audioDeviceInfo : null;

        RefreshCommand = new RelayCommand(Refresh, () => Device != null);
        RefreshCommand.CanExecuteChanged += (sender, eventArgs) => OnPropertyChanged();
    }

    private static void LogMe(bool isError, string infoLine)
    {
        if (isError)
        {
            LogManager.GetCurrentClassLogger().Error(infoLine);
        }
        else
        {
            LogManager.GetCurrentClassLogger().Info(infoLine);
        }
    }

    private static void OnDeviceAttachDetach(byte acEvent)
    {
        MyDispatcher?.Invoke(() =>
        {
            var mainWindow = Window.GetWindow(App.Current.MainWindow) as MainWindow;

            // ReSharper disable once InvertIf
            if (mainWindow?.DataContext is MainViewModel mainViewModel)
            {
                var devicePresent = acEvent is (byte)AcEvent.AcAttachedEvent or (byte)AcEvent.AcVolumeChangedEvent;
                mainViewModel.Device
                    = devicePresent
                        ? mainViewModel.AudioDeviceService.GetAudioDevice()
                        : null;
                CommandManager.InvalidateRequerySuggested();
            }
        });
    }
    public event PropertyChangedEventHandler? PropertyChanged;
    protected void OnPropertyChanged([CallerMemberName] string propertyName = "")
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public ICommand RefreshCommand { get; }


    private void Refresh()
    {
        MyDispatcher?.Invoke(() =>
        {
            if (Device != null)
            {
                Device = AudioDeviceService.GetAudioDevice();
            }
        });
    }
}

public class RelayCommand : ICommand
{
    private readonly Action _execute;
    private readonly Func<bool>? _canExecute;

    // ReSharper disable once ConvertToPrimaryConstructor
    public RelayCommand(Action execute, Func<bool>? canExecute = null)
    {
        _execute = execute ?? throw new ArgumentNullException(nameof(execute));
        _canExecute = canExecute;
    }
    public bool CanExecute(object? parameter) => _canExecute == null || _canExecute();

    public void Execute(object? parameter)
    {
        _execute();
    }

    public event EventHandler? CanExecuteChanged
    {
        add => CommandManager.RequerySuggested += value;
        remove => CommandManager.RequerySuggested -= value;
    }
}
