from distutils.core import setup

setup(
    name='TalassDB',
    version='0.1.0',
    author='Peer-Timo Bremer',
    author_email='bremer5@llnl.gov',
    packages=['talassDB'],
    package_data = {'talassDB' : ['_talassDB.so']},
    description='Python interface to TALASS statistics.',
)
