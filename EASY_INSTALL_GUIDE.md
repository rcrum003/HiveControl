# HiveControl Easy Installation Guide

**For Non-Technical Users** - Complete step-by-step instructions to get your HiveControl system running!

---

## What You'll Need

- **Raspberry Pi** (Model 3B, 3B+, 4, 5, or Zero W)
- **MicroSD Card** (16GB or larger recommended)
- **Computer** (Windows, Mac, or Linux) to prepare the SD card
- **Internet Connection** for your Raspberry Pi
- **Power Supply** for your Raspberry Pi (5V USB power adapter)
- **Optional**: Monitor, keyboard, and mouse for initial setup (or use SSH)

---

## Step 1: Prepare Your SD Card with Raspberry Pi OS

### Download Raspberry Pi Imager

The easiest way to install Raspberry Pi OS is using the official Raspberry Pi Imager tool.

1. **Download the Imager**:
   - Go to: [https://www.raspberrypi.com/software/](https://www.raspberrypi.com/software/)
   - Click the download button for your operating system (Windows, macOS, or Ubuntu)
   - Install the downloaded program

### Image Your SD Card

2. **Insert your microSD card** into your computer (you may need an SD card adapter)

3. **Open Raspberry Pi Imager**

4. **Choose Operating System**:
   - Chose Your Raspberry Device, click next
   - Choose OS
      - Select "Raspberry Pi OS (64-bit)" or "Raspberry Pi OS (32-bit)"
         - For Pi5, Pi 4 or Pi 3B+: Use 64-bit
         - For older Pi models: Use 32-bit
   - **Note**: You can use the "Lite" version (under Raspberry Pi OS (other), but you'll have to be comfortable with command line only.

5. **Choose Storage**:
   - Click "Choose Storage"
   - Select your SD card from the list
   - **WARNING**: Make sure you select the correct drive! All data on this drive will be erased.

6. **Configure Settings** (Important!):
   - Click the gear icon ⚙️ (or press Ctrl+Shift+X)
   - **Set hostname**: Enter a name like `hivecontrol` (optional but helpful) (REMEMBER THIS NAME)
   - **Enable SSH**: Check "Enable SSH" and select "Use password authentication"
   - **Set username and password**:
     - Username: `pi` (recommended, or choose your own)
     - Password: Choose a secure password and remember it!
   - **Configure WiFi** (if using WiFi):
     - Enter your WiFi network name (SSID)
     - Enter your WiFi password
     - Select your country
   - **Set locale settings**:
     - Set timezone to your location
     - Set keyboard layout to match your keyboard
   - **Enable Raspberry Connect** (Optional - for remote access):
     - Check "Enable Raspberry Connect"
     - This allows you to access your Raspberry Pi remotely from anywhere
     - You'll need a Raspberry Pi ID account (free at [raspberrypi.com](https://id.raspberrypi.com))
     - After first boot, sign in to link your device to your Raspberry Pi ID
     - Great for managing your hive remotely when away from home!
   - Click "Save"

7. **Write to SD Card**:
   - Click "Write"
   - Confirm you want to erase the SD card
   - Wait for the process to complete (5-10 minutes)
   - When done, click "Continue" and remove the SD card

---

## Step 2: Boot Your Raspberry Pi

1. **Insert the SD card** into your Raspberry Pi
2. **Connect ethernet cable** (if not using WiFi)
3. **Connect power** to start the Raspberry Pi
4. **Wait 2-3 minutes** for first boot

### Find Your Raspberry Pi on the Network

**Option A: Using a Monitor**
- Connect a monitor and keyboard to your Raspberry Pi
- You'll see the desktop after boot
- Open Terminal (black monitor icon on the taskbar)
- Skip to Step 3

**Option B: Using SSH (Headless)**
- On Windows: Download and install [PuTTY](https://www.putty.org/)
- On Mac/Linux: Open Terminal (built-in)

**Find your Pi's IP address**:
- Try connecting to the hostname: `ssh pi@hivecontrol.local` (replace hivecontrol.local with YOURHOSTNAME.local)
- OR use your router's admin page to find the IP address
- OR use a network scanner app like [Angry IP Scanner](https://angryip.org/) to find devices with port 22 and 80 open

**Connect via SSH**:
```bash
ssh pi@YOURHOSTNAME.local
# OR if using IP address:
ssh pi@192.168.1.XXX
```
- Enter the password you set earlier
- You should now see a command prompt

---

## Step 3: Install HiveControl (Easy Method)

We've made installation super simple! Just run these commands:

### One-Line Installation

Copy and paste this entire command into your terminal:

```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash
```

**What this does**:
- Downloads the installation script
- Automatically runs it with administrator privileges
- Installs all necessary software and dependencies
- Sets up the web interface
- Configures everything automatically

### Installation Options

If you need special features, use these commands instead:

**For Bee Counter Camera** (takes 6-8 hours to install):
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -b
```

**For Remote Desktop Support** (Windows users):
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -x
```

**For Touchscreen Keyboard**:
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -k
```

**For All Options**:
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -bxk
```

### What to Expect

The installation will:
1. Update your Raspberry Pi (5-10 minutes)
2. Download HiveControl from GitHub
3. Install web server (Apache and PHP)
4. Install database (SQLite)
5. Compile sensor software (20-30 minutes)
6. Set up automatic data collection
7. Ask you to set an admin password
8. Ask you to configure timezone
9. Reboot automatically

**Total time**: Between 5-60 minutes depending on the Raspberry Pi model (or 6-8 hours with Bee Counter option)

During installation, you'll see lots of text scroll by - this is normal!

---

## Step 4: Set Your Admin Password

Near the end of installation, you'll see:

```
Please set an admin password for http://127.0.0.1/admin/ access:
New password:
```

**Enter a secure password** - you'll use this to access the admin interface.

**Enter it again** when prompted to confirm.

---

## Step 5: Set Your Timezone

After setting the password, you'll see a menu to select your timezone:

1. Use arrow keys to navigate
2. Select your geographic area (e.g., "America")
3. Press Enter
4. Select your city/timezone
5. Press Enter

---

## Step 6: Reboot and Access HiveControl

After installation completes, your Raspberry Pi will automatically reboot.

Wait 1-2 minutes after reboot, then:

### Access the Web Interface

1. **Find your IP address** (shown at end of installation, or check your router)

2. **Open a web browser** on any computer on your network

3. **Go to**: `http://YOUR-PI-IP-ADDRESS/`
   - Example: `http://192.168.1.100/`
   - Or try: `http://YOURHOSTNAME.local/`

4. **Login when prompted**:
   - **IMPORTANT**: You'll immediately see a username/password prompt
   - Username: `admin`
   - Password: (the password you set during Step 4 of installation)
   - This is the admin password you created when the installer asked you to set it

5. **You should now see the First-Time Setup Wizard!**

### First-Time Setup Wizard

After logging in with your admin credentials, you'll see the setup wizard:

1. **Enter Hive Name**: Give your hive a name (e.g., "Backyard Hive 1")
   - Use only letters, numbers, dashes, and underscores
   - No spaces or special characters

2. **Enter HiveControl.org API Key** (Optional):
   - Go to [hivecontrol.org](https://hivecontrol.org) and create an account
   - Get your API key from Settings → API
   - This allows data sharing with the community

3. **Enter Location**: City and State/Province

4. **Click Save**

**Note**: The setup wizard runs on first access only. If you need to change these settings later, go to Admin → Settings → Basic.

### Configure Admin Settings

To configure sensors and advanced settings:

1. Go to: `http://YOUR-PI-IP-ADDRESS/admin/`
2. Enter username: `admin`
3. Enter the password you set during installation
4. Configure your sensors in "Settings → Instruments"

---

## Step 7: Connect Your Sensors

Now that HiveControl is installed, you can connect your sensors:

1. **Shut down your Raspberry Pi**:
   ```bash
   sudo shutdown -h now
   ```

2. **Unplug power** and wait 30 seconds

3. **Connect your sensors**:
   - Weight sensor (HX711)
   - Temperature sensors (DHT22, BME280, etc.)
   - Camera (if using)
   - See [Sensor Guide](documentation/CONFIGURATION.md) for wiring diagrams

4. **Power back on**

5. **Configure sensors**:
   - Go to Admin → Settings → Instruments
   - Enable your sensors
   - Set calibration values
   - Click "Read Sensors" to test

---

## Remote Access with Raspberry Connect (Optional)

If you enabled Raspberry Connect during SD card imaging, you can access your HiveControl system from anywhere in the world!

### What is Raspberry Connect?

Raspberry Connect is a free service from Raspberry Pi that allows you to:
- Access your Raspberry Pi remotely via web browser
- No need to configure port forwarding on your router
- Secure connection through Raspberry Pi's servers
- Access terminal, VNC, and file sharing remotely

### Setting Up Raspberry Connect

1. **Create a Raspberry Pi ID** (if you don't have one):
   - Go to [id.raspberrypi.com](https://id.raspberrypi.com)
   - Click "Create account"
   - Follow the registration process

2. **Link Your Raspberry Pi**:

   After your Pi boots for the first time, you need to sign in:

   **If using a monitor:**
   - You'll see a prompt to sign in to Raspberry Connect
   - Enter your Raspberry Pi ID credentials
   - Your device will be linked automatically

   **If using SSH (headless):**
   ```bash
   # Sign in to Raspberry Connect
   rpi-connect signin
   ```
   - Follow the on-screen instructions
   - You'll need to open a URL in your web browser
   - Enter the code shown on your Raspberry Pi
   - Confirm the linking

3. **Access Your Pi Remotely**:
   - Go to [connect.raspberrypi.com](https://connect.raspberrypi.com)
   - Sign in with your Raspberry Pi ID
   - Click on your device (e.g., "hivecontrol")
   - Choose connection type:
     - **Screen sharing** - See the desktop
     - **Shell** - Terminal access
     - **File browser** - Transfer files

### Using Raspberry Connect with HiveControl

Once connected, you can:
- Access the web interface: Open shell and run `hostname -I` to get IP, then navigate to `http://IP-ADDRESS/`
- Configure sensors remotely
- Download data backups
- Troubleshoot issues
- Update the system

### Security Note

Raspberry Connect is secure, but remember:
- Keep your Raspberry Pi ID password strong
- Enable two-factor authentication on your Raspberry Pi ID
- Only use Raspberry Connect on trusted networks
- Your Raspberry Pi connects outbound to Raspberry Pi servers (no inbound ports opened)

### Troubleshooting Raspberry Connect

**Can't sign in?**
```bash
# Check if rpi-connect is running
sudo systemctl status rpi-connect

# Restart the service
sudo systemctl restart rpi-connect

# Sign in again
rpi-connect signin
```

**Device not showing up?**
- Make sure you signed in with the same Raspberry Pi ID used during setup
- Check internet connection: `ping -c 3 raspberrypi.com`
- Wait a few minutes for device to register

**Want to disable Raspberry Connect?**
```bash
# Disable the service
sudo systemctl disable rpi-connect
sudo systemctl stop rpi-connect
```

---

## Troubleshooting

### Can't access the web interface?

1. **Verify IP address**:
   ```bash
   hostname -I
   ```

2. **Check if Apache is running**:
   ```bash
   sudo systemctl status apache2
   ```

3. **Try the local address** if you're using a monitor:
   - Go to: `http://127.0.0.1/`

### Installation failed?

1. **Check internet connection**:
   ```bash
   ping -c 3 google.com
   ```

2. **Try installing again** - it's safe to re-run

3. **Check logs**:
   ```bash
   sudo tail -50 /var/log/apache2/error.log
   ```

### Forgot admin password?

If you forgot the password you set during installation:

**Reset it with:**
```bash
cd /home/HiveControl/www/
sudo htpasswd -c .htpasswd admin
```
Enter a new password when prompted, then confirm it.

You can now login with username `admin` and your new password.

### Need more help?

- Check the [Full Documentation](README.md)
- Visit [HiveControl.org](https://hivecontrol.org) community forums
- Report issues on [GitHub](https://github.com/rcrum003/HiveControl/issues)

---

## Quick Command Reference

Here are some useful commands:

**Reboot Raspberry Pi**:
```bash
sudo reboot
```

**Shut down Raspberry Pi**:
```bash
sudo shutdown -h now
```

**Check HiveControl version**:
```bash
cat /home/HiveControl/VERSION
```

**Update HiveControl**:
```bash
sudo /home/HiveControl/upgrade.sh
```

**View system logs**:
```bash
sudo journalctl -xe
```

**Test sensors manually**:
```bash
sudo /home/HiveControl/scripts/currconditions.sh
```

---

## Next Steps

Once everything is working:

1. **Set up Raspberry Connect** (if enabled) to access your hive remotely from anywhere
2. **Monitor your dashboard** for 24 hours to ensure data is collecting
3. **Set up weather integration** (Settings → Site Configuration)
4. **Configure Growing Degree Days** if tracking bee development
5. **Set up email notifications** for alerts
6. **Join the community** at [hivecontrol.org](https://hivecontrol.org)
7. **Share your data** with [hivetool.org](https://hivetool.org) research project

---

## Maintenance

**Weekly**:
- Check that data is being collected
- Verify sensor readings are accurate

**Monthly**:
- Update HiveControl: `sudo /home/HiveControl/upgrade.sh`
- Update Raspberry Pi OS: `sudo apt update && sudo apt upgrade -y`
- Backup your database (Admin → Backup & Restore)

**Yearly**:
- Clean out old data if needed (Admin → System Commands)
- Calibrate weight sensors
- Check all sensor connections

---

## Support

**Need Help?**
- Documentation: [README.md](README.md)
- Community: [hivecontrol.org](https://hivecontrol.org)
- Issues: [GitHub Issues](https://github.com/rcrum003/HiveControl/issues)

**Found a Bug?**
Please report it with:
- HiveControl version
- Raspberry Pi model
- Raspberry Pi OS version
- Description of the problem
- Any error messages

---

**Congratulations!** You now have a fully functional HiveControl system. Happy beekeeping! 🐝
