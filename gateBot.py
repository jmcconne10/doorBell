import discord
import firebase_admin
from firebase_admin import credentials, db
import os
import asyncio
from dotenv import load_dotenv

setSleep = 10

# Load environment variables from .env file
load_dotenv()
TOKEN = os.getenv('DISCORD_BOT_TOKEN')

# Initialize Firebase with your service account credentials
cred = credentials.Certificate("serviceAccountKey.json")  # Update with the path to your JSON file
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://doorbell-338a5-default-rtdb.firebaseio.com/'
})

# Create intents object and enable necessary intents
intents = discord.Intents.default()
intents.message_content = True  # Required to read message content

client = discord.Client(intents=intents)

# Replace 'your_channel_id' with the actual ID of the channel
TARGET_CHANNEL_ID = 1298099576540237825  # Copy the channel ID here

# Function to update Firebase
def update_firebase_alert(value):
    ref = db.reference("ALERT")
    ref.set(value)  # Set the ALERT value to the provided value
    print(f"Firebase ALERT set to {value}")

@client.event
async def on_ready():
    print(f'We have logged in as {client.user}')

@client.event
async def on_message(message):
    # Check if the message is from the target channel
    if message.channel.id == TARGET_CHANNEL_ID:
        print(f"Message from {message.author}: {message.content}")
        if message.content == "!gate":
            print("Gate command received")
            update_firebase_alert(True)  # Update Firebase when !gate is received
            
            # Wait for 10 seconds, then reset the ALERT to False
            await asyncio.sleep(setSleep)
            update_firebase_alert(False)  # Reset ALERT to False after 10 seconds

# Run the bot with your token
client.run(TOKEN)
