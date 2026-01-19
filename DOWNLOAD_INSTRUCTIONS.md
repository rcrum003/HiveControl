# HiveControl Download & Installation

## Choose Your Installation Method

We offer two ways to install HiveControl - pick the one that works best for you!

---

## Method 1: One-Line Installation (Easiest!)

This is the simplest way to install HiveControl. Just copy and paste one command!

### Requirements
- Raspberry Pi with Raspberry Pi OS installed
- Internet connection
- SSH access or keyboard/monitor connected

### Installation Command

**Copy this entire command and paste it into your Raspberry Pi terminal:**

```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash
```

That's it! The installer will:
- Download everything automatically
- Install all dependencies
- Set up the web server
- Configure the database
- Compile sensor software
- Reboot when complete

**Total time**: 30-60 minutes

### With Optional Features

**Bee Counter Camera** (6-8 hours):
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -b
```

**Remote Desktop (XRDP)**:
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -x
```

**Touchscreen Keyboard**:
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -k
```

**All Features**:
```bash
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -bxk
```

---

## Method 2: Download and Run (For Users Who Prefer More Control)

If you prefer to download the script first and review it before running:

### Step 1: Download the installer

```bash
curl -O https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh
```

### Step 2: Review the script (optional)

```bash
less install.sh
```
Press 'q' to quit viewing

### Step 3: Make it executable

```bash
chmod +x install.sh
```

### Step 4: Run the installer

```bash
sudo ./install.sh
```

Or with options:
```bash
sudo ./install.sh -b    # Bee Counter
sudo ./install.sh -x    # Remote Desktop
sudo ./install.sh -k    # Touchscreen Keyboard
sudo ./install.sh -bxk  # All features
```

---

## Pre-Made SD Card Image (Coming Soon!)

We're working on creating a pre-configured SD card image that includes:
- Raspberry Pi OS
- HiveControl pre-installed
- All dependencies ready to go

Just download, flash to SD card, and boot!

**Status**: In development
**Expected release**: Check [hivecontrol.org](https://hivecontrol.org) for updates

---

## What Happens During Installation?

Here's what the installer does (so you know what to expect):

1. **System Update** (5-10 min)
   - Updates Raspberry Pi OS packages
   - Ensures you have the latest security patches

2. **Install Web Server** (2-3 min)
   - Apache web server
   - PHP and required modules
   - SQLite database

3. **Download HiveControl** (1-2 min)
   - Clones from GitHub
   - Sets up directory structure
   - Copies configuration files

4. **Install Dependencies** (10-15 min)
   - Build tools (cmake, gcc)
   - Sensor libraries (pigpio, hidapi)
   - Python packages for sensors

5. **Compile Sensor Software** (15-20 min)
   - Temperature sensor drivers
   - Weight sensor software
   - Bluetooth support for BroodMinder

6. **Configure Database** (1-2 min)
   - Creates database structure
   - Applies all patches
   - Sets initial configuration

7. **Set Up Web Server** (1-2 min)
   - Configures Apache
   - Sets file permissions
   - Creates admin interface

8. **Configure System** (2-3 min)
   - Sets up automatic data collection (cron)
   - Asks for admin password
   - Asks for timezone

9. **Final Setup** (1-2 min)
   - Installs system updates
   - Cleans up temporary files

10. **Reboot**
    - System restarts automatically
    - You're ready to go!

---

## After Installation

### Access Your HiveControl System

1. **Find your Raspberry Pi's IP address**:
   ```bash
   hostname -I
   ```

2. **Open a web browser** on any device on your network

3. **Navigate to**: `http://YOUR-PI-IP/`
   - Example: `http://192.168.1.100/`
   - Or try: `http://hivecontrol.local/`

4. **Complete the setup wizard**:
   - Enter hive name
   - Enter location
   - Add HiveControl.org API key (optional)

5. **Configure sensors**:
   - Go to Admin → Settings → Instruments
   - Enable your sensors
   - Set calibration values

---

## Troubleshooting Downloads

### Download fails or times out?

**Check internet connection**:
```bash
ping -c 3 google.com
```

**Try downloading to a different location**:
```bash
cd /home/pi
curl -O https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh
```

### curl: command not found?

Install curl first:
```bash
sudo apt update
sudo apt install curl -y
```

### Permission denied?

Make sure you're using `sudo`:
```bash
sudo bash easy-install.sh
```

---

## Alternative: Manual Git Clone

For developers or advanced users:

```bash
sudo apt update
sudo apt install git -y
cd /home
sudo git clone https://github.com/rcrum003/HiveControl.git
cd HiveControl
sudo ./install.sh
```

---

## Security Notes

- The installer requires `sudo` (administrator) access
- All code is open source and available on GitHub
- The script downloads from the official GitHub repository
- You can review the code before running if desired

**Reviewing the code**:
```bash
curl https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh | less
```
Press 'q' to exit

---

## Need Help?

- **Full Guide**: See [EASY_INSTALL_GUIDE.md](EASY_INSTALL_GUIDE.md)
- **Documentation**: [README.md](README.md)
- **Community**: [hivecontrol.org](https://hivecontrol.org)
- **Report Issues**: [GitHub Issues](https://github.com/rcrum003/HiveControl/issues)

---

## Comparison: Which Method Should I Use?

| Feature | One-Line Install | Download & Run | Manual Git Clone |
|---------|-----------------|----------------|------------------|
| **Difficulty** | Easiest | Easy | Moderate |
| **Steps** | 1 command | 4 commands | 5+ commands |
| **Best for** | First-time users | Users who want to review code | Developers |
| **Review code first?** | No | Yes | Yes |
| **Time to start** | Instant | 1 minute | 2 minutes |

**Recommendation**: Use the One-Line Install unless you have a specific reason not to!

---

## Coming Soon

We're working on even easier installation methods:

- **Pre-configured SD card image** - Flash and boot!
- **Web-based installer** - Install from a web page
- **Mobile app** - Configure from your phone
- **Auto-discovery** - Finds your Pi automatically on your network

Stay tuned at [hivecontrol.org](https://hivecontrol.org)!
