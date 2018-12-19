const { app, BrowserWindow } = require('electron');
const Types = require("bindings")("PolymorphicTypes");

let win;

function createWindow() {
  win = new BrowserWindow({ width: 800, height: 600 });

  win.loadFile('app/index.html');

  win.webContents.openDevTools();

  const function_type = Types.create_function_type();
  const function_type2 = Types.create_function_type();
  const function_type3 = function_type.compose(function_type2);
  const covariant = Types.create_covariant_type();
  function_type3.compose(covariant);

  win.on('closed', () => {
    win = null;
  });
}

app.on('ready', createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (win === null) {
    createWindow();
  }
});
