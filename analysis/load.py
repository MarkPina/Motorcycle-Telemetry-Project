import sys
import pandas as pd
import matplotlib.pyplot as plt


def load(path):
    df = pd.read_csv(path)

    expected = ['timestamp_ms', 'ax', 'ay', 'az', 'gx', 'gy', 'gz']
    missing = [c for c in expected if c not in df.columns]
    if missing:
        raise ValueError(f"missing columns: {missing}")

    df['t'] = (df['timestamp_ms'] - df['timestamp_ms'].iloc[0]) / 1000.0
    return df


def summarize(df):
    print(f"rows:     {len(df)}")
    print(f"duration: {df['t'].iloc[-1]:.1f} s")
    print(f"rate:     {len(df) / df['t'].iloc[-1]:.1f} Hz")

    gaps = df['timestamp_ms'].diff().dropna()
    print(f"dt median: {gaps.median():.1f} ms")
    print(f"dt max:    {gaps.max():.1f} ms")


def plot(df):
    fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, figsize=(10, 6))

    ax1.plot(df['t'], df['ax'], label='ax')
    ax1.plot(df['t'], df['ay'], label='ay')
    ax1.plot(df['t'], df['az'], label='az')
    ax1.set_ylabel('accel (g)')
    ax1.legend()
    ax1.grid(True)

    ax2.plot(df['t'], df['gx'], label='gx')
    ax2.plot(df['t'], df['gy'], label='gy')
    ax2.plot(df['t'], df['gz'], label='gz')
    ax2.set_ylabel('gyro (deg/s)')
    ax2.set_xlabel('time (s)')
    ax2.legend()
    ax2.grid(True)

    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    path = sys.argv[1] if len(sys.argv) > 1 else 'data/test.csv'
    df = load(path)
    summarize(df)
    plot(df)